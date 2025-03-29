#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"

namespace Ramulator {

class BATBasedRFM : public IControllerPlugin, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, BATBasedRFM, "BATBasedRFM", "BAT Based RFMs.")

private:
    IDRAM* m_dram = nullptr;
    std::vector<int> m_bank_ctrs;

    Clk_t m_clk = 0;

    int m_rfm_req_id = -1;
    int m_no_send = -1;

    int m_rank_level = -1;
    int m_bank_level = -1;
    int m_bankgroup_level = -1;
    int m_row_level = -1;
    int m_col_level = -1;

    int m_num_ranks = -1;
    int m_num_bankgroups = -1;
    int m_num_banks_per_bankgroup = -1;
    int m_num_banks_per_rank = -1;
    int m_num_rows_per_bank = -1;
    int m_num_cls = -1;

    int m_bat = -1;
    bool m_debug = false;

    bool m_enable_early_counter_reset = true;
    int m_rfm_type = -1; // 0: RFMab, 1:RFMsb, 2:RFMpb (Not part of JEDEC SPEC currently)

    // Stats
    uint64_t s_rfm_counter = 0;
    uint64_t s_early_counter_reset = 0;

public:
    void init() override { 
        m_bat = param<int>("bat").default_val(75); // Default value for PRAC in JESD79-5C 
        m_debug = param<bool>("debug").default_val(false);
        m_rfm_type = param<int>("rfm_type").default_val(0);
        m_enable_early_counter_reset = param<bool>("enable_early_counter_reset").default_val(true);
    }

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
        m_ctrl = cast_parent<IDRAMController>();
        m_dram = m_ctrl->m_dram;
        if (!m_dram->m_requests.contains("rfm")) {
            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMab" << std::endl; 
            exit(0);
        }else if (!m_dram->m_requests.contains("same-bank-rfm")) {
            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMsb" << std::endl; 
            exit(0);
        }else if (m_rfm_type == 2 && !m_dram->m_requests.contains("per-bank-rfm")) {
            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMpb" << std::endl; 
            exit(0);
        }

        switch (m_rfm_type) {
            case 0:
                m_rfm_req_id = m_dram->m_requests("rfm");
                break;
            case 1:
                m_rfm_req_id = m_dram->m_requests("same-bank-rfm");
                break;
            case 2:
                m_rfm_req_id = m_dram->m_requests("per-bank-rfm");
                break;
            default:
                std::cout<<"Wrong rfm type!!"<<std::endl;
                break;
        }

        m_rank_level = m_dram->m_levels("rank");
        m_bank_level = m_dram->m_levels("bank");
        m_bankgroup_level = m_dram->m_levels("bankgroup");
        m_row_level = m_dram->m_levels("row");
        m_col_level = m_dram->m_levels("column");

        m_num_ranks = m_dram->get_level_size("rank");
        m_num_bankgroups = m_dram->get_level_size("bankgroup");
        m_num_banks_per_bankgroup = m_dram->get_level_size("bankgroup") < 0 ? 0 : m_dram->get_level_size("bank");
        m_num_banks_per_rank = m_dram->get_level_size("bankgroup") < 0 ? 
                                m_dram->get_level_size("bank") : 
                                m_dram->get_level_size("bankgroup") * m_dram->get_level_size("bank");
        m_num_rows_per_bank = m_dram->get_level_size("row");
        m_num_cls = m_dram->get_level_size("column") / 8;
        
        m_bank_ctrs.resize(m_num_ranks * m_num_banks_per_rank);
        for (int i = 0; i < m_bank_ctrs.size(); i++) {
            m_bank_ctrs[i] = 0;
        }
        m_no_send = 0;

        register_stat(s_rfm_counter).name("num_rfm");
        register_stat(s_early_counter_reset).name("num_early_counter_reset");
    }

    int calc_flat_bank_id(ReqBuffer::iterator& req_it) {
        int flat_bank_id = req_it->addr_vec[m_bank_level];
        int accumulated_dimension = 1;
        for (int i = m_bank_level - 1; i >= m_rank_level; i--) {
            accumulated_dimension *= m_dram->m_organization.count[i + 1];
            flat_bank_id += req_it->addr_vec[i] * accumulated_dimension;
        }
        return flat_bank_id;
    }

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
        m_clk++;

        if (!request_found) {
            return;
        }

        auto& req = *req_it;
        auto& req_meta = m_dram->m_command_meta(req.command);
        auto& req_scope = m_dram->m_command_scopes(req.command);
        if (!(req_meta.is_opening && req_scope == m_row_level)) {
            return; 
        }

        int flat_bank_id = calc_flat_bank_id(req_it);
        m_bank_ctrs[flat_bank_id]++;

        if (m_debug) {
            std::cout << "Increment BAC @ "<<m_clk<<std::endl;
            std::cout << "Rank     : " << req_it->addr_vec[m_rank_level] << std::endl;
            std::cout << "BankGroup: " << req_it->addr_vec[m_bankgroup_level] << std::endl;
            std::cout << "Bank     : " << req_it->addr_vec[m_bank_level] << std::endl;
            std::cout << "Flat Bank: " << flat_bank_id << std::endl;
            std::cout << "Counter Value: " <<m_bank_ctrs[flat_bank_id]<< std::endl;
            std::cout << "Row     : " <<req_it->addr_vec[m_row_level]<< std::endl;
        }

        if (m_bank_ctrs[flat_bank_id] < m_bat) {
            return;
        }

        switch (m_rfm_type) {
            case 0:{   //RFMab
                        Request rfm(req.addr_vec, m_rfm_req_id);
                        s_rfm_counter++;
                        rfm.addr_vec[m_bankgroup_level] = -1;
                        rfm.addr_vec[m_bank_level] = -1;

                        if (!m_ctrl->priority_send(rfm)) {
                            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] Could not send request: RFMab"<<std::endl; 
                            exit(0);
                        }
                       if (m_enable_early_counter_reset) {
                           //1. Reset all bank counters if early counter reset is enabled
                           for (int i = 0; i < m_num_banks_per_rank; i++) {
                                flat_bank_id = rfm.addr_vec[m_rank_level] * m_num_banks_per_rank + i;
                                if (m_bank_ctrs[flat_bank_id] < m_bat)
                                    s_early_counter_reset++;
                                m_bank_ctrs[flat_bank_id] = 0;
                            }
                       }
                       else {
                           flat_bank_id = calc_flat_bank_id(req_it);
                           m_bank_ctrs[flat_bank_id] = 0;
                       }
                   }
                   break;
            case 1:{   //RFMsb
                       Request rfm(req.addr_vec, m_rfm_req_id);
                        s_rfm_counter++;
                        rfm.addr_vec[m_bankgroup_level] = -1;
                        if (!m_ctrl->priority_send(rfm)) {
                            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] Could not send request: RFMab"<<std::endl; 
                            exit(0);
                        }
                       if (m_enable_early_counter_reset) {
                           //1. Reset all same bank in all bank groups if early counter reset is enabled
                            for (size_t j = 0; j < m_num_bankgroups; j++){
                                flat_bank_id = rfm.addr_vec[m_bank_level] + j * m_num_banks_per_bankgroup + rfm.addr_vec[m_rank_level] * m_num_banks_per_rank;
                                if (m_bank_ctrs[flat_bank_id] < m_bat)
                                    s_early_counter_reset++;

                                m_bank_ctrs[flat_bank_id] = 0;
                            }
                       }
                       else {
                           flat_bank_id = calc_flat_bank_id(req_it);
                           m_bank_ctrs[flat_bank_id] = 0;
                       }
                   }
                   break;
            case 2:{    //RFMpb
                        Request rfm(req.addr_vec, m_rfm_req_id);
                        if (!m_ctrl->priority_send(rfm)) {
                            std::cout << "[Ramulator::BATBasedRFM] [CRITICAL ERROR] Could not send request: RFMab"<<std::endl; 
                            exit(0);
                        }
                        flat_bank_id = calc_flat_bank_id(req_it);
                        m_bank_ctrs[flat_bank_id] = 0;
                        s_rfm_counter++;
                   }
                   break;
        }
    }
};

}       // namespace Ramulator
