#include "ramulator2_wrapper.h"
#include <fmt/core.h>
#include <iostream>

Ramulator2DRAM::Ramulator2DRAM(const std::string& config_path, std::vector<channel_type*>&& ul, std::size_t rq_size, std::size_t wq_size): queues(std::move(ul)), RQ{rq_size}, WQ{wq_size}
{
    YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
    ramulator2_frontend.reset(Ramulator::Factory::create_frontend(config));
    ramulator2_memorysystem.reset(Ramulator::Factory::create_memory_system(config));

    ramulator2_frontend->connect_memory_system(ramulator2_memorysystem.get());
    ramulator2_memorysystem->connect_frontend(ramulator2_frontend.get());

    num_mem_channels = config["MemorySystem"]["DRAM"]["org"]["channel"].as<int>();
    std::cout<<"Ramulator2 is Connected"<<std::endl;

    registerExitCallback([this]() {
        ramulator2_frontend->finalize();
        ramulator2_memorysystem->finalize();
    });
}

Ramulator2DRAM::~Ramulator2DRAM() {
    finalize();
}

void Ramulator2DRAM::initialize()
{
    using namespace champsim::data::data_literals;
    using namespace std::literals::chrono_literals;

    YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
    YAML::Node dram_config = config["MemorySystem"]["DRAM"];

    // Extract number of channels
    int num_channels = num_mem_channels;
    int num_ranks = dram_config["org"]["rank"].as<int>();
    int clock_ratio = config["MemorySystem"]["clock_ratio"].as<int>();
    std::string timing_preset = dram_config["timing"]["preset"].as<std::string>();
    std::string org_preset = dram_config["org"]["preset"].as<std::string>();

    // Extract size (16Gb, 32Gb, 64Gb)
    std::size_t dram_size_per_device = 0;
    if (org_preset.find("16Gb") != std::string::npos) {
        dram_size_per_device = 16L * 1024 * 1024 * 1024;
    } else if (org_preset.find("32Gb") != std::string::npos) {
        dram_size_per_device = 32L * 1024 * 1024 * 1024;
    } else if (org_preset.find("64Gb") != std::string::npos) {
        dram_size_per_device = 64L * 1024 * 1024 * 1024;
    } else {
        fmt::print("[RAMULATOR2] WARNING: Unknown DRAM preset: {}\n", org_preset);
    }

    // Compute total DRAM size
    std::size_t total_dram_size = dram_size_per_device * num_channels;

    // Print DRAM size in human-readable format
    if (champsim::data::gibibytes gb_sz{total_dram_size}; gb_sz > 1_GiB) {
        fmt::print("Off-chip DRAM Size: {}\n", gb_sz);
    } else if (champsim::data::mebibytes mb_sz{total_dram_size}; mb_sz > 1_MiB) {
        fmt::print("Off-chip DRAM Size: {}\n", mb_sz);
    } else if (champsim::data::kibibytes kb_sz{total_dram_size}; kb_sz > 1_kiB) {
        fmt::print("Off-chip DRAM Size: {}\n", kb_sz);
    } else {
        fmt::print("Off-chip DRAM Size: {}\n", total_dram_size);
    }

    fmt::print(" Channels: {} Ranks: {} Clock Ratio: {} Timing Preset: {}\n",
               num_channels, num_ranks, clock_ratio, timing_preset);
}


void Ramulator2DRAM::initiate_requests() {
    for (auto* ul: queues) {
        // Process Read and Prefetch requests
        for (auto q: {std::ref(ul->RQ), std::ref(ul->PQ)}) {
            auto [begin, end] = champsim::get_span_p(
                    std::cbegin(q.get()), std::cend(q.get()),
                    [this, ul](const auto& pkt) { return this->add_rq(pkt, ul); }
            );
            q.get().erase(begin, end); // Remove processed requests
        }
        // Process Write requests
        auto [wq_begin, wq_end] = champsim::get_span_p(
                std::cbegin(ul->WQ), std::cend(ul->WQ),
                [this](const auto& pkt) {return this->add_wq(pkt); }
        );
        ul->WQ.erase(wq_begin, wq_end); //Remove processed write requests
    }
}


bool Ramulator2DRAM::add_rq(const request_type& packet, champsim::channel* ul){
    fmt::print("[RTQ] {} address: {} v_address: {} pf_metadata: {} time: {}\n", __func__, packet.address, packet.v_address, packet.pf_metadata,  current_time);  

    // 1. Check if there is an empty entry in RQ
    auto rq_it = std::find_if_not(std::begin(RQ), std::end(RQ), [](const auto& pkt) { return pkt.has_value(); });

    if (rq_it == std::end(RQ)) {
        fmt::print("[RTQ] RQ is FULL!\n");
        return false;
    }

    bool enqueue_success = ramulator2_frontend->receive_external_requests(
            0, packet.address, 0, [this](Ramulator::Request& req) {
                process_response(req.addr);
            });

    if (!enqueue_success) {
        fmt::print("[RTQ] Ramulator2 cannot accept requests!\n");
        return false;
    }

    // 3. Insert request
    request_type new_req(packet);
    if(packet.response_requested) {
        new_req.to_return.push_back(&ul->returned);
    }
    **rq_it = new_req;
    fmt::print("[RTQ] Send Request to Ramulator2!\n");
    return true;
}

bool Ramulator2DRAM::add_wq(const request_type& packet){
    fmt::print("[WTQ] {} address: {} v_address: {} time: {}\n", __func__, packet.address, packet.v_address, current_time);  

    // 1. Check if there is an empty entry in queue
    auto wq_it = std::find_if_not(std::begin(WQ), std::end(WQ), [](const auto& pkt) { return pkt.has_value(); });

    if (wq_it == std::end(WQ)) {
        fmt::print("[WTQ] WQ is FULL!\n");
        return false;
    }

    // 2. Check if Ramulator2 can accept the request
    bool enqueue_success = ramulator2_frontend->receive_external_requests(
            1, packet.address, 0, [this](Ramulator::Request& req) {
                process_response(req.addr);
            });

    if (!enqueue_success){
        fmt::print("[WTQ] Ramulator2 cannot accept requests!\n");
        return false;
    }
    // 3. Insert request
    **wq_it = request_type{packet};
    fmt::print("[WTQ] Send Request to Ramulator2!\n");

    return true;
}

void Ramulator2DRAM::process_response(uint64_t addr) {
    auto it = std::find_if(RQ.begin(), RQ.end(), [&](const std::optional<request_type>& req) {
            return req.has_value() && req->address == addr;
            });

    if (it != RQ.end()) {
        for (auto* ret: it->value().to_return) {
            ret->push_back(it->value()); // Send data back to LLC
            progress++;
        }
        RQ.erase(it);
    }
}

long Ramulator2DRAM::operate() {
    ramulator2_memorysystem->tick();
    initiate_requests();
    return 1; // Hardcoded 1 for now
}

void Ramulator2DRAM::finalize() {
    ramulator2_frontend->finalize();
    ramulator2_memorysystem->finalize();
}
