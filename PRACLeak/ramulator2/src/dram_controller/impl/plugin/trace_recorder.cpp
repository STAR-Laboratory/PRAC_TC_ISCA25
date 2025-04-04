#include <vector>
#include <unordered_map>
#include <limits>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "base/base.h"
#include "dram_controller/controller.h"
#include "dram_controller/plugin.h"

namespace Ramulator {

class TraceRecorder : public IControllerPlugin, public Implementation {
  RAMULATOR_REGISTER_IMPLEMENTATION(IControllerPlugin, TraceRecorder, "TraceRecorder", "CounterBasedTRR.")
  private:
    IDRAM* m_dram;

    std::filesystem::path m_trace_path; 
    Logger_t m_tracer;

    Clk_t m_clk = 0;

  public:
    void init() override { 
      m_trace_path = param<std::string>("path").desc("Path to the trace file").required();
      auto parent_path = m_trace_path.parent_path();
      std::filesystem::create_directories(parent_path);
      if (!(std::filesystem::exists(parent_path) && std::filesystem::is_directory(parent_path))) {
        throw ConfigurationError("Invalid path to trace file: {}", parent_path.string());
      }
    };

    void setup(IFrontEnd* frontend, IMemorySystem* memory_system) override {
      m_ctrl = cast_parent<IDRAMController>();
      m_dram = m_ctrl->m_dram;

      auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fmt::format("{}.ch{}", m_trace_path.string(), m_ctrl->m_channel_id), true);
      m_tracer = std::make_shared<spdlog::logger>(fmt::format("trace_recorder_ch{}", m_ctrl->m_channel_id), sink);
      m_tracer->set_pattern("%v");
      m_tracer->set_level(spdlog::level::trace);      
    };

    void update(bool request_found, ReqBuffer::iterator& req_it) override {
      m_clk++;

      if (request_found) {

        // convert addr_vec to addr
        // m_addr_bits: {0, 1, 3, 2, 16, 6}, last 6 is trimmed 
        // vaddr: [14 lost, 16, 2, 3, 1, 6, 6]
        unsigned long long addr = req_it->addr_vec[4];
        addr <<= 6;
        addr |= req_it->addr_vec[5];
        addr <<= 1;
        addr |= req_it->addr_vec[1];
        addr <<= 3;
        addr |= req_it->addr_vec[2];
        addr <<= 2;
        addr |= req_it->addr_vec[3];
        addr <<= 6;

        m_tracer->trace(
          "{}, {}, {}, {}", 
          m_clk,
          m_dram->m_commands(req_it->command),
          fmt::join(req_it->addr_vec, ", "),
          addr
        );
      }

    };

};

}       // namespace Ramulator
