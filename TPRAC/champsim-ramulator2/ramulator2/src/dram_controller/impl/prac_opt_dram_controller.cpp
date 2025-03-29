#include "dram_controller/bh_controller.h"
#include "memory_system/memory_system.h"
#include "frontend/frontend.h"
// #include "frontend/impl/processor/bhO3/bhllc.h"
// #include "frontend/impl/processor/bhO3/bhO3.h"

#include "dram_controller/impl/plugin/prac/prac.h"

namespace Ramulator {

DECLARE_DEBUG_FLAG(DBHCTRL);
ENABLE_DEBUG_FLAG(DBHCTRL);

class PRACOPTDRAMController final : public IBHDRAMController, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IBHDRAMController, PRACOPTDRAMController, "PRACOPTDRAMController", "PRAC Optimized DRAM controller.")

private:
    Logger_t m_logger;
    std::deque<Request> pending;          // A queue for read requests that are about to finish (callback after RL)
    // BHO3LLC* m_llc;
    IPRAC* m_prac;

    ReqBuffer m_active_buffer;            // Buffer for requests being served. This has the highest priority 
    ReqBuffer m_priority_buffer;          // Buffer for high-priority requests (e.g., maintenance like refresh).
    ReqBuffer m_read_buffer;              // Read request buffer
    ReqBuffer m_write_buffer;             // Write request buffer
    ReqBuffer m_prac_buffer;              // Custom PRAC buffer
    
    Request* m_prea_template;
    Request* m_rfmab_template;

    int m_rank_addr_idx = -1;
    int m_bankgroup_addr_idx = -1;
    int m_bank_addr_idx = -1;
    int m_row_addr_idx = -1;

    float m_wr_low_watermark;
    float m_wr_high_watermark;
    bool  m_is_write_mode = false;

    uint64_t s_num_read_reqs = 0;
    uint64_t s_num_write_reqs = 0;
    uint64_t s_num_other_reqs = 0;
    uint64_t s_num_refresh_reqs = 0;
    double s_num_ref_windows = 0;
    uint64_t s_num_rfm_reqs = 0;
    
    uint64_t s_queue_len = 0;
    uint64_t s_read_queue_len = 0;
    uint64_t s_write_queue_len = 0;
    uint64_t s_priority_queue_len = 0;
    
    double s_queue_len_avg = 0;
    double s_read_queue_len_avg = 0;
    double s_write_queue_len_avg = 0;
    double s_priority_queue_len_avg = 0;
        
    uint64_t s_read_latency = 0;
    double s_read_latency_avg = 0;

    uint64_t s_num_row_hits = 0;
    uint64_t s_num_row_misses = 0;
    uint64_t s_num_row_conflicts = 0;

    // DEBUG STAT
    int m_invalidate_ctr = -1;

public:
    void init() override {
        m_invalidate_ctr = 0;
        m_wr_low_watermark =  param<float>("wr_low_watermark").desc("Threshold for switching back to read mode.").default_val(0.2f);
        m_wr_high_watermark = param<float>("wr_high_watermark").desc("Threshold for switching to write mode.").default_val(0.8f);

        m_scheduler = create_child_ifce<IBHScheduler>();
        m_refresh = create_child_ifce<IRefreshManager>();
        m_rowpolicy = create_child_ifce<IRowPolicy>();

        if (m_config["plugins"]) {
            YAML::Node plugin_configs = m_config["plugins"];
            for (YAML::iterator it = plugin_configs.begin(); it != plugin_configs.end(); ++it) {
                m_plugins.push_back(create_child_ifce<IControllerPlugin>(*it));
            }
        }

        // TODO: Just create it manually / get rid of the plugin and inject it here.
        m_prac = get_plugin<IPRAC>();
        if (!m_prac) {
            std::cout << "[PRACCTRL] Need PRAC plugin!";
            std::exit(0);
        }
    };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
        // m_llc = static_cast<BHO3*>(frontend)->get_llc();
        m_dram = memory_system->get_ifce<IDRAM>();
        m_rank_addr_idx = m_dram->m_levels("rank");
        m_bankgroup_addr_idx = m_dram->m_levels("bankgroup");
        m_bank_addr_idx = m_dram->m_levels("bank");
        m_row_addr_idx = m_dram->m_levels("row");

        m_logger = Logging::create_logger(fmt::format("DBHCTRL_{}", m_channel_id));

        std::vector<int> all_bank_addr_vec(m_dram->m_levels.size(), -1);
        all_bank_addr_vec[m_dram->m_levels("channel")] = m_channel_id;
        int m_prea_id = m_dram->m_commands("PREA");
        int m_rfmab_id = m_dram->m_commands("RFMab");
        
        m_prea_template = new Request(all_bank_addr_vec, m_dram->m_requests("close-all-bank"));
        m_prea_template->command = m_prea_id;
        m_prea_template->final_command = m_prea_id;

        m_rfmab_template = new Request(all_bank_addr_vec, m_dram->m_requests("rfm"));
        m_rfmab_template->command = m_rfmab_id;
        m_rfmab_template->final_command = m_rfmab_id;
        
        // TODO: change this later considering per-bank command queue size
        m_active_buffer.max_size = INT_MAX;
        m_priority_buffer.max_size = INT_MAX;
        m_prac_buffer.max_size = INT_MAX;

        register_stat(s_num_row_hits).name("controller{}_num_row_hits", m_channel_id);
        register_stat(s_num_row_misses).name("controller{}_num_row_misses", m_channel_id);
        register_stat(s_num_row_conflicts).name("controller{}_num_row_conflicts", m_channel_id);
        
        register_stat(s_num_read_reqs).name("controller{}_num_read_reqs", m_channel_id);
        register_stat(s_num_write_reqs).name("controller{}_num_write_reqs", m_channel_id);
        register_stat(s_num_other_reqs).name("controller{}_num_other_reqs", m_channel_id);
        
        register_stat(s_num_refresh_reqs).name("controller{}_num_refresh_reqs", m_channel_id);
        register_stat(s_num_ref_windows).name("controller{}_num_ref_windows", m_channel_id);
        
        register_stat(s_num_rfm_reqs).name("controller{}_num_rfm_reqs", m_channel_id);

        register_stat(s_read_latency).name("controller{}_read_latency", m_channel_id);
        register_stat(s_read_latency_avg).name("controller{}_read_latency_avg", m_channel_id);
        
        register_stat(s_queue_len).name("controller{}_queue_len", m_channel_id);
        register_stat(s_read_queue_len).name("controller{}_read_queue_len", m_channel_id);
        register_stat(s_write_queue_len).name("controller{}_write_queue_len", m_channel_id);
        register_stat(s_priority_queue_len).name("controller{}_priority_queue_len", m_channel_id);
        
        register_stat(s_queue_len_avg).name("controller{}_queue_len_avg", m_channel_id);
        register_stat(s_read_queue_len_avg).name("controller{}_read_queue_len_avg", m_channel_id);
        register_stat(s_write_queue_len_avg).name("controller{}_write_queue_len_avg", m_channel_id);
        register_stat(s_priority_queue_len_avg).name("controller{}_priority_queue_len_avg", m_channel_id);
    };

    bool send(Request& req) override {
        req.final_command = m_dram->m_request_translations(req.type_id);
        
        // Forward existing write requests to incoming read requests
        if (req.type_id == Request::Type::Read) {
            auto compare_addr = [req](const Request& wreq) {
                return wreq.addr == req.addr;
            };
            if (std::find_if(m_write_buffer.begin(), m_write_buffer.end(), compare_addr) != m_write_buffer.end()) {
                // The request will depart at the next cycle
                req.depart = m_clk + 1;
                pending.push_back(req);
                return true;
            }
        }
        // Else, enqueue them to corresponding buffer based on request type id
        bool is_success = false;
        req.arrive = m_clk;
        if        (req.type_id == Request::Type::Read) {
            is_success = m_read_buffer.enqueue(req);
        } else if (req.type_id == Request::Type::Write) {
            is_success = m_write_buffer.enqueue(req);
        } else {
            throw std::runtime_error("Invalid request type!");
        }
        if (!is_success) {
            // We could not enqueue the request
            req.arrive = -1;
            return false;
        }
        return true;
    };

    bool priority_send(Request& req) override {
        req.final_command = m_dram->m_request_translations(req.type_id);

        bool is_success = false;
        is_success = m_priority_buffer.enqueue(req);
        return is_success;
    }

    bool is_all_bank_case(const auto& req) {
      return req.addr_vec[3] == -1 && req.addr_vec[2] == -1;
    }

    bool is_same_bank_case(const auto& req) {
      return req.addr_vec[3] == -1 && req.addr_vec[2] != -1;
    }

    // Command conflict checker with debug feature
    bool has_conflict(const auto& req, const auto& it, int command_id) {
        bool conflict = false;

        // Dereference it and req to access addr_vec
        if (is_all_bank_case(*it) && (*it).addr_vec[1] == (*req).addr_vec[1]) {
            conflict = true;
        }
        else if (is_same_bank_case(*it) && (*it).addr_vec[1] == (*req).addr_vec[1] && (*it).addr_vec[3] == (*req).addr_vec[3]) {
            conflict = true;
        }
        else if ((*it).command == command_id &&
                (*it).addr_vec[1] == (*req).addr_vec[1] && 
                (*it).addr_vec[2] == (*req).addr_vec[2] && 
                (*it).addr_vec[3] == (*req).addr_vec[3]) {
            conflict = true;
        }

        // Debug output
        // #ifdef DEBUG
        // if (conflict) {
        //     std::cout << "Conflict detected at clock cycle " << m_clk << ":\n";
        //     std::cout << "  Command: " << (*it).command << "\n";
        //     std::cout << "  Request Addr Vec: [ " << (*req).addr_vec[0] << ", " << (*req).addr_vec[1]
        //               << ", " << (*req).addr_vec[2] << ", " << (*req).addr_vec[3] << " ]\n";
        //     std::cout << "  Iterator Addr Vec: [ " << (*it).addr_vec[0] << ", " << (*it).addr_vec[1]
        //               << ", " << (*it).addr_vec[2] << ", " << (*it).addr_vec[3] << " ]\n";
        //     std::cout << "  Conflict Type: ";
        //     if (is_all_bank_case(*it)) std::cout << "All Bank\n";
        //     else if (is_same_bank_case(*it)) std::cout << "Same Bank\n";
        //     else std::cout << "Direct Match\n";
        // }
        // #endif

        return conflict;
    }

    void tick() override {
        m_clk++;

        // Update stats
        s_queue_len += m_read_buffer.size() + m_write_buffer.size() + m_priority_buffer.size() + pending.size();
        s_read_queue_len += m_read_buffer.size() + pending.size();
        s_write_queue_len += m_write_buffer.size();
        s_priority_queue_len += m_priority_buffer.size();

        // Serve completed reads
        serve_completed_reads();

        m_refresh->tick();
        m_scheduler->tick();

        // Do we need to setup for the ABO recovery period?
        bool is_recovery_starting = m_prac->next_recovery_cycle() - m_clk <= m_dram->m_timing_vals("nRP") + 5;
        bool is_recovery_setup = m_prac_buffer.size() != 0;
        if (is_recovery_starting && !is_recovery_setup) {
            for (int i = 0; i < m_dram->get_level_size("rank"); i++) {
                m_prea_template->addr_vec[m_dram->m_levels("rank")] = i;
                m_prac_buffer.enqueue(*m_prea_template);
            }
            for (int i = 0; i < m_prac->get_num_abo_recovery_refs(); i++) {
                // Alternate ranks, as PRIO/PRAC queue is served FCFS
                for (int j = 0; j < m_dram->get_level_size("rank"); j++) {
                    m_rfmab_template->addr_vec[m_dram->m_levels("rank")] = j;
                    m_prac_buffer.enqueue(*m_rfmab_template);
                }
            }
        }

        // Try to find a request to serve.
        ReqBuffer::iterator req_it;
        ReqBuffer* buffer = nullptr;
        bool request_found = schedule_request(req_it, buffer);

        // RowPolicy
        m_rowpolicy->update(request_found, req_it);

        // Update all plugins
        for (auto plugin : m_plugins) {
            plugin->update(request_found, req_it);
        }

        // 4. Finally, issue the commands to serve the request
        if (request_found) {
            // If we find a real request to serve
            m_dram->issue_command(req_it->command, req_it->addr_vec);

            // If we are issuing the last command, set depart clock cycle and move the request to the pending queue
            if (req_it->command == req_it->final_command) {
                if (req_it->type_id == Request::Type::Read) {
                    req_it->depart = m_clk + m_dram->m_read_latency;
                    pending.push_back(*req_it);
                    s_num_read_reqs++;
                } else if (req_it->type_id == Request::Type::Write) {
                    s_num_write_reqs++;
                }
                else{
                    s_num_other_reqs++;
                    if (req_it->command == m_dram->m_commands("REFab") || req_it->command == m_dram->m_commands("REFsb")){
                            s_num_refresh_reqs++;
                    }
                    if (req_it->command == m_dram->m_commands("RFMab") || req_it->command == m_dram->m_commands("RFMsb") || req_it->command == m_dram->m_commands("RFMpb")){
                            s_num_rfm_reqs++;
                    }
                }
                buffer->remove(req_it);
            }else {
                if (m_dram->m_command_meta(req_it->command).is_opening) {
                    bool is_success = m_active_buffer.enqueue(*req_it);
                    if (is_success)
                        buffer->remove(req_it);
                }
            }
        }
    };

private:
    /**
        * @brief    Helper function to serve the completed read requests
        * @details
        * This function is called at the beginning of the tick() function.
        * It checks the pending queue to see if the top request has received data from DRAM.
        * If so, it finishes this request by calling its callback and poping it from the pending queue.
        */
    void serve_completed_reads() {
        if (pending.size()) {
            // Check the first pending request
            auto& req = pending[0];
            if (req.depart <= m_clk) {
                // Request received data from dram
                if (req.depart - req.arrive > 1) {
                    // Check if this requests accesses the DRAM or is being forwarded.
                    s_read_latency += req.depart - req.arrive;
                }

                if (req.callback) {
                    // If the request comes from outside (e.g., processor), call its callback
                    req.callback(req);
                }
                // Finally, remove this request from the pending queue
                pending.pop_front();
            }
        };
    };


    /**
        * @brief    Checks if we need to switch to write mode
        * 
        */
    void set_write_mode() {
        if (!m_is_write_mode) {
            if ((m_write_buffer.size() > m_wr_high_watermark * m_write_buffer.max_size) || m_read_buffer.size() == 0) {
                m_is_write_mode = true;
            }
        } else {
            if ((m_write_buffer.size() < m_wr_low_watermark * m_write_buffer.max_size) && m_read_buffer.size() != 0) {
                m_is_write_mode = false;
            }
        }
    };

    /**
        * @brief    Helper function to find a request to schedule from the buffers.
        * 
        */
    bool schedule_request(ReqBuffer::iterator& req_it, ReqBuffer*& req_buffer) {
        bool request_found = false;
        Clk_t next_recovery_clk = m_prac->next_recovery_cycle();
        // 2.1    First, check the act buffer to serve requests that are already activating (avoid useless ACTs)
        if (req_it = m_scheduler->get_best_request(m_active_buffer); req_it != m_active_buffer.end()) { 
            bool fits = m_clk + m_prac->min_cycles_with_preall(req_it) < next_recovery_clk;
            if (fits && m_dram->check_ready(req_it->command, req_it->addr_vec)) {
                request_found = true;
                req_buffer = &m_active_buffer;
            }
        }
        // 2.2    If no requests can be scheduled from the act buffer, check the rest of the buffers
        if (!request_found) {
            // 2.2.1    We first check if MC has critical ABO requests
            if (m_prac_buffer.size() != 0) {
                req_buffer = &m_prac_buffer;
                req_it = m_prac_buffer.begin();
                req_it->command = m_dram->get_preq_command(req_it->final_command, req_it->addr_vec);

                bool is_rfm = req_it->command == m_dram->m_commands("RFMab");
                bool is_pre_rec = m_prac->get_state() == IPRAC::ABOState::PRE_RECOVERY;
                bool early_issue = is_rfm && is_pre_rec; // Prevent controller from issuing RFMab before recovery starts
                request_found = !early_issue && m_dram->check_ready(req_it->command, req_it->addr_vec);
                // if (!request_found && m_prac_buffer.size() != 0){
                //     return false;
                // }
            }

            // 2.2.2    We then check the priority buffer to prioritize e.g., maintenance requests
            if (m_priority_buffer.size() != 0) {
                if (request_found){
                    auto& buffer = m_priority_buffer;
                    auto temp_req_it = m_scheduler->get_best_request(buffer);
                    if (temp_req_it != buffer.end()){
                        if (temp_req_it->final_command == m_dram->m_commands("REFab")){
                            request_found = m_dram->check_ready(req_it->command, req_it->addr_vec);
                            bool is_same_rank = temp_req_it->addr_vec[1] == req_it->addr_vec[1];
                            if (request_found && is_same_rank){
                                req_it = temp_req_it;
                                req_buffer = &m_priority_buffer;
                            }
                            else if (!request_found){
                                if (is_same_rank)
                                    return false;
                            }
                        }                 
                    }
                }
                else{
                    auto& buffer = m_priority_buffer;
                    if (req_it = m_scheduler->get_best_request(buffer); req_it != buffer.end()){
                        bool fits = m_clk + m_prac->min_cycles_with_preall(req_it) < next_recovery_clk;
                        if (req_it->final_command == m_dram->m_commands("REFab")){
                            request_found = m_dram->check_ready(req_it->command, req_it->addr_vec);
                            req_buffer = &m_priority_buffer;
                        }
                        else {
                            request_found = fits && m_dram->check_ready(req_it->command, req_it->addr_vec);
                            req_buffer = &m_priority_buffer;     
                        }                       
                    }
                    if (!request_found && m_prac_buffer.size() != 0){
                        return false;
                    }
                }
                // auto& buffer = m_priority_buffer;
                // if (req_it = m_scheduler->get_best_request(buffer); req_it != buffer.end()){
                //     bool fits = m_clk + m_prac->min_cycles_with_preall(req_it) < next_recovery_clk;
                //     request_found = fits && m_dram->check_ready(req_it->command, req_it->addr_vec);
                //     req_buffer = &m_priority_buffer;
                // }
            }

            if (!request_found) {
                // Query the write policy to decide which buffer to serve
                set_write_mode();
                auto& buffer = m_is_write_mode ? m_write_buffer : m_read_buffer;
                req_it = m_scheduler->get_best_request(buffer);
                if (req_it != buffer.end()) {
                    // 1. Check if command is ready
                    bool fits = m_clk + m_prac->min_cycles_with_preall(req_it) < next_recovery_clk;
                    request_found = fits && m_dram->check_ready(req_it->command, req_it->addr_vec);
                    req_buffer = &buffer;
                    // 2. Check if there is any RFM or Refresh conflit
                    if (request_found && m_priority_buffer.size() != 0) {
                        for (auto _it = m_priority_buffer.begin(); _it != m_priority_buffer.end(); _it++) {
                            if (has_conflict(req_it, _it, m_dram->m_commands("VRR")) || has_conflict(req_it, _it, m_dram->m_commands("RFMpb"))){
                                request_found = false;
                                req_buffer = &m_priority_buffer;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (request_found && m_dram->m_command_meta(req_it->command).is_closing) {
            auto& rowgroup = req_it->addr_vec;
            for (auto _it = m_active_buffer.begin(); _it != m_active_buffer.end(); _it++) {
                auto& _it_rowgroup = _it->addr_vec;
                bool is_matching = true;
                for (int i = 0; i < m_bank_addr_idx + 1 ; i++) {
                    if (_it_rowgroup[i] != rowgroup[i] && _it_rowgroup[i] != -1 && rowgroup[i] != -1) {
                        is_matching = false;
                        break;
                    }
                }
                if (is_matching) {
                    request_found = false;
                    break;
                }
            }
        }

        if (request_found && req_buffer != &m_active_buffer) {
            if (req_it->type_id == Request::Type::Read
                || req_it->type_id == Request::Type::Write) {
                auto& req_meta = m_dram->m_command_meta(req_it->command);
                if (req_meta.is_accessing) {
                    s_num_row_hits++;
                }
                if (req_meta.is_opening) {
                    s_num_row_misses++;
                }
                if (req_meta.is_closing) {
                    s_num_row_conflicts++;
                }
            }
        }
        return request_found;
    }

    void finalize() override {
        s_read_latency_avg = (double)s_read_latency / (double)s_num_read_reqs;

        s_queue_len_avg = (double)s_queue_len / (double)m_clk;
        s_read_queue_len_avg = (double)s_read_queue_len / (double)m_clk;
        s_write_queue_len_avg = (double)s_write_queue_len / (double)m_clk;
        s_priority_queue_len_avg = (double)s_priority_queue_len / (double)m_clk;

        s_num_ref_windows = (double)s_num_refresh_reqs / (double) (8192*m_dram->get_level_size("rank")); // Assuming 8K REFs per tREFW
    }
};
}   // namespace Ramulator