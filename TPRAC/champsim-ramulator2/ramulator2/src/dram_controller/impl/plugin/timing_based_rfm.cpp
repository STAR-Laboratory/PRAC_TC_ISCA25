#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"
#include <cmath>

namespace Ramulator {

class TimingBasedRFM : public IControllerPlugin, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, TimingBasedRFM, "TimingBasedRFM", "Timing Based RFMs.")

private:
    IDRAM* m_dram = nullptr;
    std::vector<int> m_bank_ctrs;

    Clk_t m_clk = 0;

    int m_dram_org_levels = -1;
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

    float m_num_rfm_per_tREFI = 0.0;
    int m_TB_RFM_window_ns = -1;
    Clk_t m_TB_RFM_window_clk = 0;

    int m_targeted_ref_ratio = -1;

    Clk_t m_next_rfm_cycle = -1;
    Clk_t m_next_targeted_ref_cycle = -1;
    Clk_t m_targeted_ref_cycle = -1;
    Clk_t m_nrefi = -1;
    Clk_t m_nrfc = -1;
    Clk_t m_nrfmab = -1;
    Clk_t m_nrfmi = -1;
    Clk_t m_tref_skipped_interval = -1;

    int m_rfm_per_trefi_cnt = -1;

    bool m_require_rfm = false;

    bool m_debug = false;

    int m_rfm_type = -1; // 0: RFMab, 1:RFMsb, 2:RFMpb (Not part of JEDEC SPEC currently)

    // Stats
    uint64_t s_rfm_counter = 0;

public:
    void init() override { 
        // m_num_rfm_per_tREFI = param<float>("num_rfm_per_tREFI").default_val(1.0);  // # of RFMs per tREFI
        m_TB_RFM_window_ns = param<int>("TB_RFM_window").default_val(3900);  // # TB_RFM_Window in ns
        m_targeted_ref_ratio = param<int>("targeted_ref_ratio").default_val(0); // Targeted refresh ratio X every tREFI
        m_debug = param<bool>("debug").default_val(false);
        m_rfm_type = param<int>("rfm_type").default_val(0);
    }

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
        m_ctrl = cast_parent<IDRAMController>();
        m_dram = m_ctrl->m_dram;

        if (!m_dram->m_requests.contains("rfm")) {
            std::cout << "[Ramulator::TimingBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMab" << std::endl; 
            exit(0);
        }else if (!m_dram->m_requests.contains("same-bank-rfm")) {
            std::cout << "[Ramulator::TimingBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMsb" << std::endl; 
            exit(0);
        }else if (m_rfm_type == 2 && !m_dram->m_requests.contains("per-bank-rfm")) {
            std::cout << "[Ramulator::TimingBasedRFM] [CRITICAL ERROR] DRAM Device does not support request: RFMpb" << std::endl; 
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
        
        m_dram_org_levels = m_dram->m_levels.size();

        m_num_ranks = m_dram->get_level_size("rank");
        m_num_bankgroups = m_dram->get_level_size("bankgroup");
        m_num_banks_per_bankgroup = m_dram->get_level_size("bankgroup") < 0 ? 0 : m_dram->get_level_size("bank");
        m_num_banks_per_rank = m_dram->get_level_size("bankgroup") < 0 ? 
                                m_dram->get_level_size("bank") : 
                                m_dram->get_level_size("bankgroup") * m_dram->get_level_size("bank");
        m_num_rows_per_bank = m_dram->get_level_size("row");
        m_num_cls = m_dram->get_level_size("column") / 8;

        m_nrefi = m_dram->m_timing_vals("nREFI");
        m_nrfc = m_dram->m_timing_vals("nRFC1");
        m_nrfmab = m_dram->m_timing_vals("nRFM1");

        // if (m_num_rfm_per_tREFI < 1.0){
        //     m_nrfmi = static_cast<int>(std::round(m_nrefi / m_num_rfm_per_tREFI));
        //     m_next_rfm_cycle = m_nrfmi-m_nrfmab - m_nrfc;
        // }
        // else{
        //     float temp_nrfmi = (float) (m_nrefi - m_nrfc) / m_num_rfm_per_tREFI;
        //     m_nrfmi = static_cast<int>(std::round(temp_nrfmi));
        //     m_next_rfm_cycle = m_nrfmi-m_nrfmab;
        // }

        m_TB_RFM_window_clk = m_TB_RFM_window_ns / ((float) m_dram->m_timing_vals("tCK_ps") / 1000.0f); 
        m_next_rfm_cycle = m_TB_RFM_window_clk - m_nrfmab;
        if(m_next_rfm_cycle <= 0){
            std::printf("[ERROR] TB_RFM_window should be larger than 350ns!\n");
            std::abort();  // Immediately terminates the program
        }
        m_targeted_ref_cycle = m_targeted_ref_ratio * m_nrefi;
        m_next_targeted_ref_cycle = m_targeted_ref_cycle;
        //** If Targeted Refresh is more frequent than Timing-based RFM, then we just rely on targeted refresh **/
        if (m_targeted_ref_cycle!=0 && m_targeted_ref_cycle <= m_next_rfm_cycle){
            m_next_rfm_cycle = 0;
        }

        m_no_send = 0;
        m_require_rfm = true;
        m_tref_skipped_interval = 0;
        register_stat(s_rfm_counter).name("num_rfm");

        if (m_debug){
            std::printf("nREFI: %ld, nRFC1: %ld\n", m_nrefi, m_nrfc);
            std::printf("RFM Interval in Clock: %ld, Next RFM Cycle: %ld, Next Targeted REF Cycle: %ld\n", m_TB_RFM_window_clk, m_next_rfm_cycle, m_next_targeted_ref_cycle);
        }
    }

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
        m_clk++;
        if (m_next_rfm_cycle >0 && m_targeted_ref_cycle != 0 && m_clk == m_next_targeted_ref_cycle){
            if(m_debug){
                std::printf("[TimingBasedRFM][Targeted REF] Targeted Refresh is issued @ %ld\n",m_clk);
            }                 
            m_next_targeted_ref_cycle += m_targeted_ref_cycle;
            // Clk_t rfm_tref_interval = m_next_rfm_cycle - m_clk;
            // m_tref_skipped_interval += (m_TB_RFM_window_clk - rfm_tref_interval);
            // if (m_tref_skipped_interval <= m_TB_RFM_window_clk){
            //     m_require_rfm = false;
            // }
            // else{
            //     m_require_rfm = true;
            // }
            // if(m_debug){
            //     if(!m_require_rfm){
            //         std::printf("[TimingBasedRFM][Targeted REF] Will skip next RFM. Current Skipped interval: %ld\n", m_tref_skipped_interval);
            //     }
            //     else{
            //         std::printf("[TimingBasedRFM][Targeted REF] Will perform RFM next. Current Skipped interval: %ld\n", m_tref_skipped_interval);
            //     }
            // }


            // if (m_next_rfm_cycle < m_clk){
            //     std::printf("[ERROR] TB RFM must have been issued before targeted REF!");
            //     std::abort();
            // }
            // m_next_targeted_ref_cycle += m_targeted_ref_cycle;
            // Clk_t new_next_rfm_cycle = m_next_rfm_cycle + (m_TB_RFM_window_clk - (m_clk - (m_next_rfm_cycle - m_TB_RFM_window_clk)));
            // if(m_debug){
            //     std::printf("[TimingBasedRFM][Targeted REF] Targeted Refresh is issued @ %ld Next Targeted REF: %ld Previous next_rfm_cycle: %ld New next_rfm_cycle: %ld\n",m_clk, m_next_targeted_ref_cycle, m_next_rfm_cycle, new_next_rfm_cycle);
            // }
            // m_next_rfm_cycle = new_next_rfm_cycle;

        }

        if (m_next_rfm_cycle > 0 && m_clk == m_next_rfm_cycle) {
            bool is_skip_rfm = false;
            if(m_targeted_ref_ratio > 0){
                Clk_t temp_next_rfm_cycle = m_clk + m_TB_RFM_window_clk;
                // 1. Check if we there will be targeted refresh before next rfm cycle
                if(temp_next_rfm_cycle >= m_next_targeted_ref_cycle){
                    // 1. Check if we can skip next targeted ref or not
                    if(m_tref_skipped_interval < m_TB_RFM_window_clk){
                        m_tref_skipped_interval += m_next_targeted_ref_cycle - m_clk;
                        m_next_rfm_cycle += m_TB_RFM_window_clk;
                        if(m_debug){
                            std::printf("[TimingBasedRFM] Skip current RFM @ %ld. Next RFM Cycle: %ld. Next Targeted REF: %ld. Skipped Interval: %ld\n",m_clk, m_next_rfm_cycle, m_next_targeted_ref_cycle, m_tref_skipped_interval);
                        }
                        is_skip_rfm = true;
                    } else {
                        m_tref_skipped_interval -= m_TB_RFM_window_clk;
                        if(m_debug){
                            std::printf("[TimingBasedRFM] Cannot skip current RFM due to skipped interval @ %ld. Next RFM Cycle: %ld. Next Targeted REF: %ld. Skipped Interval: %ld\n",m_clk, m_next_rfm_cycle, m_next_targeted_ref_cycle, m_tref_skipped_interval);
                        }
                    }
                }
            }
            if (!is_skip_rfm){
                for (int rank = 0; rank < m_num_ranks; rank++){
                    std::vector<int> addr_vec(m_dram_org_levels, -1);
                    addr_vec[0] = m_ctrl->m_channel_id;
                    addr_vec[1] = rank;
                    Request rfm(addr_vec, m_rfm_req_id);
                    if(m_debug){
                        std::printf("[TimingBasedRFM] [Sending RFM] @ [%lu] Ra: %d\n", m_clk, rank);
                    }
                    if (!m_ctrl->priority_send(rfm)) {
                        std::cout << "[Ramulator::TimingBasedRFM] [CRITICAL ERROR] Could not send request: RFMab"<<std::endl; 
                        exit(0);
                    }               
                    s_rfm_counter++;
                }
                m_next_rfm_cycle += m_TB_RFM_window_clk;
            }
        }
    }
};

}       // namespace Ramulator
