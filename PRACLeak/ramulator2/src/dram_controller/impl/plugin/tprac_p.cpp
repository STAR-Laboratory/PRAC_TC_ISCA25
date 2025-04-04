#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"
#include <cmath>
#include <random>

namespace Ramulator {

class TPRAC_P : public IControllerPlugin, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, TPRAC_P, "TPRAC_P", "Timing Based RFMs with randomization.")

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
    float m_random_rfm_probability = 0.0;

    Clk_t m_next_rfm_cycle = -1;
    Clk_t m_next_refresh_cycle = -1;
    int m_nrefi = -1;
    int m_nrfc = -1;
    int m_nrfmab = -1;
    Clk_t m_nrfmi = -1;

    int m_rfm_per_trefi_cnt = -1;

    bool m_debug = false;

    int m_rfm_type = -1; // 0: RFMab, 1:RFMsb, 2:RFMpb (Not part of JEDEC SPEC currently)
    // rng for random policy
    std::mt19937 generator;
    std::uniform_real_distribution<double> distribution;
    // Stats
    uint64_t s_rfm_counter = 0;

public:
    void init() override { 
        m_num_rfm_per_tREFI = param<float>("num_rfm_per_tREFI").default_val(1.0);  // # of RFMs per tREFI
        m_random_rfm_probability = param<float>("random_rfm_probability").default_val(1.0);  // # of RFMs per tREFI
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

        if (m_num_rfm_per_tREFI < 1.0){
            m_nrfmi = static_cast<int>(std::round(m_nrefi / m_num_rfm_per_tREFI));
            m_next_rfm_cycle = m_nrfmi-m_nrfmab - m_nrfc;

        }
        else{
            float temp_nrfmi = (float) (m_nrefi - m_nrfc) / m_num_rfm_per_tREFI;
            m_nrfmi = static_cast<int>(std::round(temp_nrfmi));
            m_next_rfm_cycle = m_nrfmi-m_nrfmab;
        }
        m_rfm_per_trefi_cnt = 0;

        m_next_refresh_cycle = m_nrefi / 2;
        m_no_send = 0;

        register_stat(s_rfm_counter).name("num_rfm");

        if (m_debug){
            std::printf("nREFI: %d, nRFC1: %d\n", m_nrefi, m_nrfc);
            std::printf("RFM Interval in Clock: %lu, Next RFM Cycle: %lu \n", m_nrfmi, m_next_rfm_cycle);
        }

        // Step 1: Initialize the Mersenne Twister generator with a fixed seed
        generator = std::mt19937(1337);

        // Step 2: Define a uniform real distribution between 0 and 1
        distribution =  std::uniform_real_distribution<double>(0.0, 1.0);

    }

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
        m_clk++;

        if (m_clk == m_next_rfm_cycle) {
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
            m_next_rfm_cycle += m_nrfmi;
            if (m_nrfmi <= m_nrefi){
                //1. Perform 1+ RFMs per tREFI
                m_rfm_per_trefi_cnt++;
                if (m_rfm_per_trefi_cnt % (int)m_num_rfm_per_tREFI == 0){
                    m_next_rfm_cycle += m_nrfc;
                }
            }
        }
        // For the random rfm
        if (m_clk == m_next_refresh_cycle) {
            for (int rank = 0; rank < m_num_ranks; rank++) {
                // Step 3: Generate a random number
                double random_value = distribution(generator);
                if (random_value < m_random_rfm_probability){
                    std::vector<int> addr_vec(m_dram_org_levels, -1);
                    addr_vec[0] = m_ctrl->m_channel_id;
                    addr_vec[1] = rank;
                    Request rfm(addr_vec, m_rfm_req_id);
                    if(m_debug){
                        std::printf("[TimingBasedRFM] [Sending RAND RFM] @ [%lu] Ra: %d, Probability: %f\n", m_clk, rank, random_value);
                    }
                    if (!m_ctrl->priority_send(rfm)) {
                        std::cout << "[Ramulator::TimingBasedRFM] [CRITICAL ERROR] Could not send request: RFMab"<<std::endl; 
                        exit(0);
                    }               
                    s_rfm_counter++;
                }
            }
            m_next_refresh_cycle += m_nrefi;
        }
    }
};

}       // namespace Ramulator
