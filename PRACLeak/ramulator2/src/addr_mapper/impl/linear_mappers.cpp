#include <vector>

#include "base/base.h"
#include "dram/dram.h"
#include "addr_mapper/addr_mapper.h"
#include "memory_system/memory_system.h"
#include <bitset>

namespace Ramulator {

class LinearMapperBase : public IAddrMapper {
  public:
    IDRAM* m_dram = nullptr;

    int m_num_levels = -1;          // How many levels in the hierarchy?
    std::vector<int> m_addr_bits;   // How many address bits for each level in the hierarchy?
    Addr_t m_tx_offset = -1;

    int m_col_bits_idx = -1;
    int m_row_bits_idx = -1;


  protected:
    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) {
      m_dram = memory_system->get_ifce<IDRAM>();

      // Populate m_addr_bits vector with the number of address bits for each level in the hierachy
      const auto& count = m_dram->m_organization.count;
      m_num_levels = count.size();
      m_addr_bits.resize(m_num_levels);
      for (size_t level = 0; level < m_addr_bits.size(); level++) {
        m_addr_bits[level] = calc_log2(count[level]);
      }

      // Last (Column) address have the granularity of the prefetch size
      m_addr_bits[m_num_levels - 1] -= calc_log2(m_dram->m_internal_prefetch_size);

      int tx_bytes = m_dram->m_internal_prefetch_size * m_dram->m_channel_width / 8;
      m_tx_offset = calc_log2(tx_bytes);

      // Determine where are the row and col bits for ChRaBaRoCo and RoBaRaCoCh
      try {
        m_row_bits_idx = m_dram->m_levels("row");
      } catch (const std::out_of_range& r) {
        throw std::runtime_error(fmt::format("Organization \"row\" not found in the spec, cannot use linear mapping!"));
      }

      // Assume column is always the last level
      m_col_bits_idx = m_num_levels - 1;
    }

};


class ChRaBaRoCo final : public LinearMapperBase, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IAddrMapper, ChRaBaRoCo, "ChRaBaRoCo", "Applies a trival mapping to the address.");

  public:
    void init() override { };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      LinearMapperBase::setup(frontend, memory_system);
    }

    void apply(Request& req) override {
      req.addr_vec.resize(m_num_levels, -1);
      Addr_t addr = req.addr >> m_tx_offset;
      for (int i = m_addr_bits.size() - 1; i >= 0; i--) {
        req.addr_vec[i] = slice_lower_bits(addr, m_addr_bits[i]);
      }
    }
};


class RoCoRaBaCh final : public LinearMapperBase, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IAddrMapper, RoCoRaBaCh, "RoCoRaBaCh", "Applies a RoCoRaBaCh mapping to the address.");

  public:
    void init() override { };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      LinearMapperBase::setup(frontend, memory_system);
    }

    void apply(Request& req) override {
      req.addr_vec.resize(m_num_levels, -1);
      Addr_t addr = req.addr >> m_tx_offset;

      // Channel
      req.addr_vec[0] = slice_lower_bits(addr, m_addr_bits[0]);
      // Bank group
      req.addr_vec[3] = slice_lower_bits(addr, m_addr_bits[3]);
      // Bank
      req.addr_vec[2] = slice_lower_bits(addr, m_addr_bits[2]);
      // Rank
      req.addr_vec[1] = slice_lower_bits(addr, m_addr_bits[1]);
      // Column
      req.addr_vec[5] = slice_lower_bits(addr, m_addr_bits[5]);
      // Row
      req.addr_vec[4] = slice_lower_bits(addr, m_addr_bits[4]);
    }
};


class RoBaRaCoCh final : public LinearMapperBase, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IAddrMapper, RoBaRaCoCh, "RoBaRaCoCh", "Applies a RoBaRaCoCh mapping to the address.");

  public:
    void init() override { };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      LinearMapperBase::setup(frontend, memory_system);
    }

    void apply(Request& req) override {
      req.addr_vec.resize(m_num_levels, -1);
      Addr_t addr = req.addr >> m_tx_offset;
      req.addr_vec[0] = slice_lower_bits(addr, m_addr_bits[0]);
      req.addr_vec[m_addr_bits.size() - 1] = slice_lower_bits(addr, m_addr_bits[m_addr_bits.size() - 1]);
      for (int i = 1; i <= m_row_bits_idx; i++) {
        req.addr_vec[i] = slice_lower_bits(addr, m_addr_bits[i]);
      }
    }
};


class MOP4CLXOR final : public LinearMapperBase, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IAddrMapper, MOP4CLXOR, "MOP4CLXOR", "Applies a MOP4CLXOR mapping to the address.");

  public:
    void init() override { };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      LinearMapperBase::setup(frontend, memory_system);
    }

    void apply(Request& req) override {
      req.addr_vec.resize(m_num_levels, -1);
      // std::cout << "Binary: " << std::bitset<64>(req.addr) << std::endl; // Adjust size (e.g., 8, 16, 32) as needed

      Addr_t addr = req.addr >> m_tx_offset;

      req.addr_vec[m_col_bits_idx] = slice_lower_bits(addr, 2);
      for (int lvl = 0 ; lvl < m_row_bits_idx ; lvl++){
          req.addr_vec[lvl] = slice_lower_bits(addr, m_addr_bits[lvl]);
      }
      req.addr_vec[m_col_bits_idx] += slice_lower_bits(addr, m_addr_bits[m_col_bits_idx]-2) << 2;
      // std::printf("addr after 2nd col: %d\n", addr);
      Addr_t row_addr = addr;
      req.addr_vec[m_row_bits_idx] = slice_lower_bits(row_addr, m_addr_bits[m_row_bits_idx]);

      int row_xor_index = 0; 
      for (int lvl = 0 ; lvl < m_row_bits_idx ; lvl++){
        if (m_addr_bits[lvl] > 0){
          int mask = (req.addr_vec[m_row_bits_idx] >> row_xor_index) & ((1<<m_addr_bits[lvl])-1);
          // std::cout<<"LV: "<<lvl<<" First: "<<std::bitset<17>(req.addr_vec[m_row_bits_idx] >> row_xor_index)<<"second: "<<std::bitset<5>(1<<m_addr_bits[lvl] -1)<<std::endl;
          // std::cout<< "Mask: " << std::bitset<17>(mask)<<std::endl; // Adjust size (e.g., 8, 16, 32) as needed
          // std::cout<<"addr_vec:" << std::bitset<5>(req.addr_vec[lvl])<<std::endl; // Adjust size (e.g., 8, 16, 32) as needed

          req.addr_vec[lvl] = req.addr_vec[lvl] xor mask;
          row_xor_index += m_addr_bits[lvl];
        }
      }
      // std::printf("DRAM ADDR: CH: %d, RA: %d, BG: %d, BA: %d , Row: %d, COL %d\n", req.addr_vec[0], req.addr_vec[1], req.addr_vec[2], req.addr_vec[3], req.addr_vec[4], req.addr_vec[5]);
    }
};

}   // namespace Ramulator