#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"
#include "dram_controller/impl/plugin/prac/prac.h"
#include "dram_controller/impl/plugin/device_config/device_config.h"

#include <limits>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace Ramulator {

class QPRAC : public IControllerPlugin, public Implementation, public IPRAC {
    RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, QPRAC, "QPRAC", "PRAC Inplementation with Priority Service Queue.")

private:
    class PerBankCounters;

private:
    DeviceConfig m_cfg;
    std::vector<QPRAC::PerBankCounters> m_bank_counters;
    std::vector<int> m_same_bank_offsets;

    Clk_t m_clk = 0;

    ABOState m_state = ABOState::NORMAL;
    Clk_t m_abo_recovery_start = std::numeric_limits<Clk_t>::max();

    int m_abo_act_ns = -1;
    int m_abo_recovery_refs = -1;
    int m_abo_delay_acts = -1;
    int m_abo_thresh = -1;

    int m_abo_act_cycles = -1;

    uint32_t m_abo_recov_rem_refs = -1;
    uint32_t m_abo_delay_rem_acts = -1;
    bool m_is_abo_needed = false;

    bool m_debug = false;

    uint32_t m_psq_size = 0;
    uint32_t m_enqueuing_th = 0;
    uint32_t m_proactive_mitigation_th = 0;

    // For mitigations under retention refreshes (Targeted Refresh) -- JESD79-5C.01
    uint32_t m_targeted_ref_frequency = 0; // How often do we perform targeted ref? Ex) Once per tREFI

    bool m_enable_opportunistic_mitigation = false; // To study the impact of opportunistic mitigations

    // Stats 
    uint64_t s_num_recovery = 0; // # of ABOs
    uint64_t s_num_targeted_ref = 0; // # of total Targeted REF

    uint64_t s_psq_len = 0;
    double s_avg_psq_len = 0.0;

    // Stats for energy/power
    uint64_t s_num_total_mitigations = 0;
    double qprac_static_power = 0.380784;  //mW -- Based on Synopsys DC with 45nm Nangate Open Cell Library
    double s_qprac_total_dynamic_energy = 0.0; //nJ
    double s_qprac_total_static_energy = 0.0;  //nJ
    double s_qprac_total_mitigation_energy = 0.0;  //nJ

public:
    void init() override { 
        m_debug = param<bool>("debug").default_val(false);
        m_abo_delay_acts = param<int>("abo_delay_acts").default_val(4);
        m_abo_recovery_refs = param<int>("abo_recovery_refs").default_val(4);
        m_abo_act_ns = param<int>("abo_act_ns").default_val(180);
        m_abo_thresh = param<int>("abo_threshold").default_val(512);

        // For Priority Service Queue configurations
        m_psq_size = param<uint32_t>("psq_size").default_val(5);
        m_enqueuing_th = param<uint32_t>("enqueuing_th").default_val(1);
        m_proactive_mitigation_th = param<uint32_t>("proactive_mitigation_th").default_val(0);
        // For Targeted Refresh
        m_targeted_ref_frequency = param<uint32_t>("targeted_ref_frequency").default_val(1);
    
        m_enable_opportunistic_mitigation = param<bool>("enable_opportunistic_mitigation").default_val(true);
    }

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
        m_cfg.set_device(cast_parent<IDRAMController>());
        init_dram_params(m_cfg.m_dram);

        m_is_abo_needed = false;
        m_abo_act_cycles = m_abo_act_ns / ((float) m_cfg.m_dram->m_timing_vals("tCK_ps") / 1000.0f);
        register_stat(s_num_targeted_ref).name("num_targeted_ref");
        register_stat(s_num_total_mitigations).name("num_total_mitigations");

        register_stat(s_psq_len).name("psq_len");
        register_stat(s_avg_psq_len).name("avg_psq_len");
        // Stats for power/energy
        register_stat(s_qprac_total_dynamic_energy).name("qprac_dynamic_energy");
        register_stat(s_qprac_total_static_energy).name("qprac_static_energy");
        register_stat(s_qprac_total_mitigation_energy).name("qprac_mitigation_energy");

        m_bank_counters.reserve(m_cfg.m_num_banks);
        for (int i = 0; i < m_cfg.m_num_banks; i++) {
            m_bank_counters.emplace_back(i, m_cfg, m_is_abo_needed, m_abo_thresh, m_debug, m_psq_size, m_enqueuing_th, m_proactive_mitigation_th, m_targeted_ref_frequency, m_enable_opportunistic_mitigation, s_num_total_mitigations, s_num_targeted_ref, s_qprac_total_dynamic_energy);
        }

        register_stat(s_num_recovery).name("prac_num_recovery");
    }

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
        m_clk++;
        
        update_state_machine(request_found, *req_it);

        for (size_t i = 0; i < m_cfg.m_num_banks; i++) {
            s_psq_len += m_bank_counters[i].print_psq_size();
        }

        if (!request_found) {
            return;
        }

        auto& req = *req_it;
        auto& req_meta = m_cfg.m_dram->m_command_meta(req.command);
        auto& req_scope = m_cfg.m_dram->m_command_scopes(req.command);

        bool has_bank_wildcard = req.addr_vec[m_cfg.m_bank_level] == -1;
        bool has_bankgroup_wildcard = req.addr_vec[m_cfg.m_bankgroup_level] == -1;
        if (has_bankgroup_wildcard && has_bank_wildcard) { // All BG, All Bank
            int offset = req.addr_vec[m_cfg.m_rank_level] * m_cfg.m_num_banks_per_rank;
            for (int i = 0; i < m_cfg.m_num_banks_per_rank; i++) {
                m_bank_counters[offset + i].on_request(req);
            }
            req.addr_vec[m_cfg.m_bank_level] = -1;
        }
        else if (has_bankgroup_wildcard) { // All BG, Single Bank
            int rank_offset = req.addr_vec[m_cfg.m_rank_level] * m_cfg.m_num_banks_per_rank;
            int bank_offset = req.addr_vec[m_cfg.m_bank_level];
            for (int i = 0; i < m_cfg.m_num_bankgroups; i++) {
                int bg_offset = i * m_cfg.m_num_banks_per_bankgroup;
                m_bank_counters[rank_offset + bg_offset + bank_offset].on_request(req);
            }
        }
        else if (has_bank_wildcard) { // Single BG, All Bank
            int rank_offset = req.addr_vec[m_cfg.m_rank_level] * m_cfg.m_num_banks_per_rank;
            int bg_offset = req.addr_vec[m_cfg.m_bankgroup_level] * m_cfg.m_num_banks_per_bankgroup; 
            for (int i = 0; i < m_cfg.m_num_banks_per_bankgroup; i++) {
                m_bank_counters[rank_offset + bg_offset + i].on_request(req);
            }
        }
        else { // Single BG, Single Bank
            auto flat_bank_id = m_cfg.get_flat_bank_id(req);
            m_bank_counters[flat_bank_id].on_request(req);
        }
    }

    void update_state_machine(bool request_found, const Request& req) {
        std::unordered_map<ABOState, std::string> state_names = {
            {ABOState::NORMAL, "ABOState::NORMAL"},
            {ABOState::PRE_RECOVERY, "ABOState::PRE_RECOVERY"},
            {ABOState::RECOVERY, "ABOState::RECOVERY"},
            {ABOState::DELAY, "ABOState::DELAY"}
        };
        auto cmd_prea = m_cfg.m_dram->m_commands("PREA");
        auto cmd_rfmab = m_cfg.m_dram->m_commands("RFMab");
        auto cmd_rfmsb = m_cfg.m_dram->m_commands("RFMsb");
        auto cmd_act = m_cfg.m_dram->m_commands("ACT");
        auto cur_state = m_state;
        switch(m_state) {
        case ABOState::NORMAL:
            if (m_is_abo_needed) {
                if (m_debug) {
                    std::printf("[PRAC] [%lu] <%s> Asserting ALERT_N.\n", m_clk, state_names[cur_state].c_str());
                }
                m_state = ABOState::PRE_RECOVERY;
                m_abo_recovery_start = m_clk + m_abo_act_cycles;
                s_num_recovery++;
            }
            break;
        case ABOState::PRE_RECOVERY:
            if (request_found && req.command == cmd_prea) {
                if (m_debug) {
                    std::printf("[PRAC] [%lu] <%s> Received PREA.\n", m_clk, state_names[cur_state].c_str());
                }
            }
            if (m_clk == m_abo_recovery_start) {
                m_state = ABOState::RECOVERY;
                m_abo_recovery_start = std::numeric_limits<Clk_t>::max();
                m_abo_recov_rem_refs = m_abo_recovery_refs * m_cfg.m_num_ranks;
            }
            break;
        case ABOState::RECOVERY:
            if (request_found && (req.command == cmd_rfmab ||
                req.command == cmd_rfmsb)) {
                m_abo_recov_rem_refs--;
                if (!m_abo_recov_rem_refs) {
                    m_state = ABOState::DELAY;
                    m_abo_delay_rem_acts = m_abo_delay_acts;
                }
            }
            break;
        case ABOState::DELAY:
            if (request_found && req.command == cmd_act) {
                m_abo_delay_rem_acts--;
                if (!m_abo_delay_rem_acts) {
                    m_is_abo_needed = false;
                    for (int i = 0; i < m_cfg.m_num_banks; i++) {
                        m_is_abo_needed |= m_bank_counters[i].is_critical();
                    }
                    m_state = ABOState::NORMAL;
                }
            }
            break;
        }
        if (m_debug && cur_state != m_state) {
            std::printf("[PRAC] [%lu] <%s> -> <%s>\n", m_clk, state_names[cur_state].c_str(), state_names[m_state].c_str());
        }
    }

    Clk_t next_recovery_cycle() override {
        return m_abo_recovery_start;
    }

    int get_num_abo_recovery_refs() override {
        return m_abo_recovery_refs;
    }

    ABOState get_state() override {
        return m_state;
    }

    void finalize() override {
        s_avg_psq_len = (double)s_psq_len / (double)m_clk;
        s_avg_psq_len = s_avg_psq_len / (double)m_cfg.m_num_banks;

        // Calculate the QPRAC energy consumption here
        double VDD = m_cfg.m_dram->m_voltage_vals("VDD");
        double VPP = m_cfg.m_dram->m_voltage_vals("VPP");

        double IDD0 = m_cfg.m_dram->m_current_vals("IDD0");
        double IPP0 = m_cfg.m_dram->m_current_vals("IPP0");

        double IDD2N = m_cfg.m_dram->m_current_vals("IDD2N");
        double IPP2N = m_cfg.m_dram->m_current_vals("IPP2N");

        double IDD3N = m_cfg.m_dram->m_current_vals("IDD3N");
        double IPP3N = m_cfg.m_dram->m_current_vals("IPP3N");

        double tCK_ns = (double) m_cfg.m_dram->m_timing_vals("tCK_ps") / 1000.0;
        double tRAS = (double) m_cfg.m_dram->m_timing_vals("nRAS") * tCK_ns;
        double tRP = (double) m_cfg.m_dram->m_timing_vals("nRP") * tCK_ns;
        double tRC = tRAS + tRP;
        double mitigation_energy = ((((VDD * (IDD0 - IDD3N) + VPP * (IPP0 - IPP3N))) * tRAS)
                                    + ((VDD * (IDD0 - IDD2N) + VPP * (IPP0 - IPP2N)) * tRP)) / 1E3;
        // Each mitigation refresh/reset five rows.
        mitigation_energy = (double)mitigation_energy * 5.0;

        s_qprac_total_static_energy += (double)qprac_static_power * (double)m_clk * tCK_ns * m_cfg.m_num_ranks / 1E3;
        s_qprac_total_mitigation_energy = (double)s_num_total_mitigations * (double)mitigation_energy;
    }

private:
    class PerBankCounters {
    public: 
        PerBankCounters(int bank_id, DeviceConfig& cfg, bool& is_abo_needed, int alert_thresh, bool debug, uint32_t psq_size, uint32_t enqueuing_th, uint32_t proactive_mitigation_th, uint32_t targeted_ref_frequency, bool enable_opportunistic_mitigation, uint64_t& num_total_mitigations, uint64_t& num_targeted_ref, double& qprac_total_dynamic_energy)
        : m_bank_id(bank_id), m_cfg(cfg), m_is_abo_needed(is_abo_needed),
        m_alert_thresh(alert_thresh), m_debug(debug), m_psq_size(psq_size), m_enqueuing_th(enqueuing_th), m_proactive_mitigation_th(proactive_mitigation_th),
        m_targeted_ref_frequency(targeted_ref_frequency), m_enable_opportunistic_mitigation(enable_opportunistic_mitigation), s_num_total_mitigations(num_total_mitigations), s_num_targeted_ref(num_targeted_ref), s_qprac_total_dynamic_energy(qprac_total_dynamic_energy){
            init_dram_params(m_cfg.m_dram);
            reset();
        }

        ~PerBankCounters() {
            m_counters.clear();
        }

        void on_request(const Request& req) {
            if (m_handlertable.find(req.command) != m_handlertable.end()) {
                m_handlertable[req.command].handler(req);
            }
        }

        void init_dram_params(IDRAM* dram) {
            CommandHandler handlers[] = {
                // TODO: We should process PREs? Doesn't really change the results though.
                {std::string("ACT"), std::bind(&PerBankCounters::process_act, this, std::placeholders::_1)},
                {std::string("RFMab"), std::bind(&PerBankCounters::process_rfm, this, std::placeholders::_1)},
                {std::string("RFMsb"), std::bind(&PerBankCounters::process_rfm, this, std::placeholders::_1)},
                {std::string("REFab"), std::bind(&PerBankCounters::process_targeted_ref, this, std::placeholders::_1)}
            };
            for (auto& h : handlers) {
                if (!dram->m_commands.contains(h.cmd_name)) {
                    std::cout << "[PRAC] Command " << h.cmd_name << "does not exist." << std::endl;
                    exit(0);
                }
                m_handlertable[dram->m_commands(h.cmd_name)] = h;
            }
        }

        void reset() {
            m_counters.clear();
            m_critical_rows.clear();
            m_psq.clear();
        }

        bool is_critical() {
            return m_critical_rows.size() > 0;
        }

        uint64_t print_psq_size(){
            if(m_psq.empty())
                return 0;

            return m_psq.size();
        }

    private:
        struct CommandHandler {
            std::string cmd_name;
            std::function<void(const Request&)> handler;
        };

        DeviceConfig& m_cfg;
        bool& m_is_abo_needed;

        std::unordered_map<int, uint32_t> m_counters;
        std::unordered_map<int, uint32_t> m_critical_rows;
        std::unordered_map<int, CommandHandler> m_handlertable;

        int m_alert_thresh = -1;
        bool m_debug = false;
        int m_bank_id = -1;

        // For PSQ and configuration
        std::unordered_map<int, uint32_t> m_psq;
        uint32_t m_psq_size = 0;
        uint32_t m_enqueuing_th = 0;
        uint32_t m_proactive_mitigation_th = 0;

        bool m_enable_opportunistic_mitigation = true;
       
        // For Targeted REF
        uint32_t m_targeted_ref_frequency = 0;
        uint64_t m_num_ref = 0;

        // For stats
        uint64_t& s_num_targeted_ref;
        
        // For power related stats
        uint64_t& s_num_total_mitigations;
        double& s_qprac_total_dynamic_energy;
        double qprac_per_bank_access_energy = 0.000236893; //nJ -- Based on Synopsys DC with 45nm Nangate Open Cell Library
        
        // Functions for PSQ managements
        bool is_psq_full() {
            if(m_psq.size() > m_psq_size){
                std::printf("Error: PSQ size %ld is larger than the assigned value %ld\n", m_psq.size(), m_psq_size);
                assert(m_psq.size() <= m_psq_size);
            }

            if(m_psq.size() == m_psq_size){
                // if(m_debug)
                //     std::printf("Bank %d, PSQ is Full! Current Size: %d\n", m_bank_id, m_psq.size());
                return true;
            }
            // if(m_debug)
            //     std::printf("Bank %d, PSQ is not Full! Current Size: %d\n",m_bank_id, m_psq.size());
       
            return false;
        }

        bool replace_psq_entry(auto row_addr) {
            auto min_entry = std::min_element(m_psq.begin(), m_psq.end(), 
                    [](const auto& lhs, const auto& rhs) {
                        return lhs.second < rhs.second;
                    });
            // if (m_debug){
            //     std::printf("Current Minimum Row Id: %d, Cnt: %d\n", min_entry->first, min_entry->second);
            //     std::printf("Accesed Row Id: %d, Cnt: %d\n", row_addr, m_counters[row_addr]);
            // }
            // Replacement is performed --> Return true;
            if (m_counters[row_addr] > min_entry->second) {
                if (m_debug)
                    std::printf("Replace Row %d with Row %d\n", min_entry->first, row_addr);
                m_psq[row_addr] = m_counters[row_addr];
                if (min_entry->second >= m_alert_thresh){
                    // Remove this entry from critical entry since this row cannot be tracked from DRAM
                    m_critical_rows.erase(min_entry->first);
                }
                m_psq.erase(min_entry->first);
                return true;
            }
            return false;
        }

        // Return values:
        // 0: If it's internally incremented, 1: If it's inserted w/o replacement 
        // -1: If it's not inserted, 2: If it's inserted w/ replacement
        int update_psq(auto row_addr) {
            // 1. Check if entry is already in the PSQ
            if (m_psq.find(row_addr) != m_psq.end()) {
                m_psq[row_addr]++;
                return 0;
            }
            else{
                // 2.1 Check if counter reaches enqueueing threshold
                if (m_counters[row_addr] < m_enqueuing_th) {
                    return - 1;
                }
                else {
                    //2.2 Insert or replace the entry
                    s_qprac_total_dynamic_energy += qprac_per_bank_access_energy;
                    //2.2.1 Check if queue has an empty entry
                    if(!is_psq_full()) {
                        //2.2.2 Insert the entry into the empty space
                        m_psq[row_addr] = m_counters[row_addr];
                        return 1;
                    }
                    else {
                        //2.2.3 Replace the entry
                        if(replace_psq_entry(row_addr))
                            return 2;
                        else
                            return -1;
                    }
                }
            }
        }
        // Increase the victim rows' counters at each mitigation
        void increase_victim_counters(auto row_addr) {
            // 1. Update upper counter vlaues
            for (int up = 1 ; up <=2; up++){
                auto top_victim = row_addr - up;
                if (top_victim < 0) {
                    continue;
                }
                else {
                    if(m_counters.find(top_victim) == m_counters.end()) {
                        m_counters[top_victim] = 0;
                    }
                    m_counters[top_victim]++;
                    // If current row is not inserted into psq then do nothing
                    int update_type = update_psq(top_victim);
                    if (update_type == -1){
                        return;
                    }
                    else if(update_type >= 0 && update_type <=2){
                        // Asserts Alert if counter reaches NBO and the row is in PSQ
                        if (m_counters[top_victim] >= m_alert_thresh) {
                            // PSQ Counter should be the same as in-DRAM counter
                            if (m_counters[top_victim] != m_psq[top_victim]){
                                std::printf("[TOP VICTIM UPDATE][ERROR!] PSQ counter: %lu, and In-DRAM counter: %lu, Have Different Values!\n", m_counters[top_victim], m_psq[top_victim]);
                                assert(m_counters[top_victim] == m_psq[top_victim]);
                            }
                            m_critical_rows[top_victim] = m_counters[top_victim];
                            m_is_abo_needed = true;
                        }
                    }
                }
            }
            //2. Update bottom counter values
            for (int down = 1 ; down <=2; down++) {
                auto bottom_victim = row_addr - down;
                if (bottom_victim >= m_cfg.m_num_rows_per_bank) {
                    continue;
                }
                else {
                    if(m_counters.find(bottom_victim) == m_counters.end()){
                        m_counters[bottom_victim] = 0;
                    }
                    m_counters[bottom_victim]++;
                    int update_type = update_psq(bottom_victim);

                    // If current row is not inserted into psq then do nothing
                    if (update_type == -1){
                        return;
                    }
                    else if(update_type >= 0 && update_type <=2){
                        // Asserts Alert if counter reaches NBO and the row is in PSQ
                        if (m_counters[bottom_victim] >= m_alert_thresh) {
                            // PSQ Counter should be the same as in-DRAM counter
                            if (m_counters[bottom_victim] != m_psq[bottom_victim]){
                                std::printf("[BOTTOM VICTIM UPDATE][ERROR!] PSQ counter: %lu, and In-DRAM counter: %lu, Have Different Values!\n", m_counters[bottom_victim], m_psq[bottom_victim]);
                                assert(m_counters[bottom_victim] == m_psq[bottom_victim]);
                            }
                            m_critical_rows[bottom_victim] = m_counters[bottom_victim];
                            m_is_abo_needed = true;
                        }
                    }
                }
            }
        }
        // PSQ Mitigations -- Mitigation Type 0: RFM, 1: Targeted REF
        void process_psq_mitigation(int mitigation_type) {
            auto row_addr = -1;
            auto max_entry = std::max_element(m_psq.begin(), m_psq.end(),
                [] (const std::pair<int, uint32_t>& p1, const std::pair<int, uint32_t>& p2) {
                    return p1.second < p2.second;
                });
            // 1.Check if PSQ is empty
            if (max_entry == m_psq.end()){
                return;
            }

            // 1.1 Only for the RFMs, check if opportunistic mitigation is enabled
            if (mitigation_type == 0 && !m_enable_opportunistic_mitigation && m_counters[max_entry->first] < m_alert_thresh){
                if (m_debug){
                    std::printf("No Opportunitic Mitigation: Skips Ba: %d Cnt: %lu\n", m_bank_id, m_counters[max_entry->first]);
                }
                return;
            }
            // 1.2. Only for the targeted refreshes (proactive mitigations)
            if (mitigation_type == 1 && m_counters[max_entry->first] < m_proactive_mitigation_th){
                if (m_debug){
                    std::printf("[TARGETED REF]: MAX CTR is smaller than mitigation threshold! --> Skips Ba: %d Cnt: %lu\n", m_bank_id, m_counters[max_entry->first]);
                }
                return;               
            }
            // 2. Perform mitigations
            // 2.1 Reset counter value and remove the entry from psq
            m_counters[max_entry->first] = 0;
            row_addr = max_entry->first;
            m_critical_rows.erase(max_entry->first);
            m_psq.erase(max_entry->first);

            s_num_total_mitigations++;
            s_qprac_total_dynamic_energy += qprac_per_bank_access_energy;
            // 2.2 Perform mitigations on victims --> Increase victim counters
            increase_victim_counters(row_addr);
        }

        void process_targeted_ref (const Request& req) {
            // 1. Check if Targeted REF is enabled  
            if (m_targeted_ref_frequency == 0)
                return;
            
            m_num_ref++;
            // 2. Check if this refresh is for Targeted REF
            if (m_num_ref % m_targeted_ref_frequency != 0)
                return;
            // 3. Perform mitigation for top-most activated row
            process_psq_mitigation(1);
            if(m_bank_id == 0)
                s_num_targeted_ref += m_cfg.m_num_ranks;
        }

        void process_act(const Request& req) {
            auto row_addr = req.addr_vec[m_cfg.m_row_level];    
            if (m_counters.find(row_addr) == m_counters.end()) {
                m_counters[row_addr] = 0;
            }
            m_counters[row_addr]++;
            // if (m_debug) {
            //     std::printf("[PRAC] [%d] [ACT] Row: %d Act: %u\n",
            //         m_bank_id, row_addr, m_counters[row_addr]);
            // }
            // If current row is not inserted into psq then do nothing
            int update_type = update_psq(row_addr);

            if (update_type == -1){
                return;
            }
            else if(update_type >= 0 && update_type <= 2){
                // Asserts Alert if counter reaches NBO and the row is in PSQ
                if (m_counters[row_addr] >= m_alert_thresh) {
                    // PSQ Counter should be the same as in-DRAM counter
                    if (m_counters[row_addr] != m_psq[row_addr]){
                        std::printf("[ERROR!] PSQ counter: %lu, and In-DRAM counter: %lu, Have Different Values!\n", m_counters[row_addr], m_psq[row_addr]);
                        assert(m_counters[row_addr] == m_psq[row_addr]);
                    }
                    m_critical_rows[row_addr] = m_counters[row_addr];
                    m_is_abo_needed = true;
                }
            }
            else {
                std::printf("[ERROR!] PSQ Update Returns Wrong Value!\n");
                assert(update_type <= 2);
            }
        }

        void process_rfm(const Request& req) {
            process_psq_mitigation(0);
        }
    };  // class PerBankCounters

};      // class QPRAC

}       // namespace Ramulator
