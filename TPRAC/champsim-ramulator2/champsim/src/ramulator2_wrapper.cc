#include "ramulator2_wrapper.h"
#include <fmt/core.h>
#include <algorithm>
#include <cfenv>
#include <cmath>
#include <iostream>
#include "deadlock.h"
#include "instruction.h"
#include "util/bits.h" // for lg2, bitmask
#include "util/span.h"
#include "util/units.h"

Ramulator2DRAM::Ramulator2DRAM(champsim::chrono::picoseconds clock_period, const std::string& config_path, std::vector<channel_type*>&& ul, std::size_t rq_size, std::size_t wq_size): 
champsim::operable(clock_period),queues(std::move(ul)), clock_period(clock_period)
{
    YAML::Node config = Ramulator::Config::parse_config_file(config_path, {});
    ramulator2_frontend.reset(Ramulator::Factory::create_frontend(config));
    ramulator2_memorysystem.reset(Ramulator::Factory::create_memory_system(config));

    ramulator2_frontend->connect_memory_system(ramulator2_memorysystem.get());
    ramulator2_memorysystem->connect_frontend(ramulator2_frontend.get());

    num_mem_channels = config["MemorySystem"]["DRAM"]["org"]["channel"].as<int>();
    std::cout<<"Ramulator2 is Connected"<<std::endl;
    RQ.resize(rq_size * num_mem_channels);
    fmt::print("RQ Size: {}\n", RQ.size());
    initialize(config);
}

Ramulator2DRAM::~Ramulator2DRAM() {
    finalize();
}

void Ramulator2DRAM::initialize(YAML::Node config)
{
    using namespace champsim::data::data_literals;
    using namespace std::literals::chrono_literals;

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
    total_dram_size = dram_size_per_device * num_channels * num_ranks;

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


bool Ramulator2DRAM::add_rq(const champsim::channel::request_type& packet, champsim::channel* ul){
    // fmt::print("[RTQ] {} address: {} v_address: {} pf_metadata: {} current: {}\n", 
    //     __func__, 
    //     packet.address.to<uint64_t>(),    // Convert address to uint64_t
    //     packet.v_address.to<uint64_t>(),  // Convert v_address to uint64_t
    //     packet.pf_metadata,               // Assuming pf_metadata is directly printable
    //     (current_time.time_since_epoch()) / clock_period // 
    // );
    
    // 1. Check if there is an empty entry in RQ
    auto rq_it = std::find_if_not(std::begin(RQ), std::end(RQ), [](const auto& pkt) { return pkt.has_value(); });

    if (rq_it == std::end(RQ)) {
        // fmt::print("[RTQ] RQ is FULL!\n");
        return false;
    }

    bool enqueue_success = ramulator2_frontend->receive_external_requests(
            0, static_cast<long>(packet.address.to<long>()), 0, [this](Ramulator::Request& req) {
                process_response(req.addr);
            });

    if (!enqueue_success) {
        // fmt::print("[RTQ] Ramulator2 cannot accept requests!\n");
        return false;
    }

    // 3. Insert request
    request_type new_req(packet);
    *rq_it = new_req;
    rq_it->value().forward_checked = false;
    if(packet.response_requested) {
        rq_it->value().to_return = {&ul->returned};
    }
    // fmt::print("[RTQ] Send Request to Ramulator2!\n");
    return true;
}

bool Ramulator2DRAM::add_wq(const champsim::channel::request_type& packet){
    // fmt::print("[WRITE] {} address: 0x{:x} v_address: 0x{:x} pf_metadata: {} current: {}\n", 
    //     __func__, 
    //     packet.address.to<uint64_t>(),    // Convert address to uint64_t
    //     packet.v_address.to<uint64_t>(),  // Convert v_address to uint64_t
    //     packet.pf_metadata,               // Assuming pf_metadata is directly printable
    //     (current_time.time_since_epoch()) / clock_period // Convert to seconds since epoch
    // );
    
    // 1. Check if Ramulator2 can accept the request
    bool enqueue_success = ramulator2_frontend->receive_external_requests(
            1, static_cast<long>(packet.address.to<long>()), 0, [this](Ramulator::Request& req) {
                // process_write_response(req.addr);
            });

    if (!enqueue_success){
        // fmt::print("[WRITE] Ramulator2 cannot accept requests!\n");
        return false;
    }

    return true;
}

bool Ramulator2DRAM::is_collision(champsim::address a, champsim::address b) const {
    //// Debug print
    // fmt::print("[COLLISION_CHECK] Address a: {} Address b: {}\n", a, b);

    // if (a==b){
    //     fmt::print("Address Match: Address a: {} Address b: {} current: {}\n",
    //             a, b, (current_time.time_since_epoch()) / clock_period ); 
    // }
    return a == b;
}

void Ramulator2DRAM::check_read_collision() {
    for (auto rq_it = std::begin(RQ); rq_it != std::end(RQ); ++rq_it) {
        if (rq_it->has_value() && !rq_it->value().forward_checked) {
            auto checker = [this, check_val = rq_it->value().address](const auto& x) {
                return x.has_value() && this->is_collision(x.value().address, check_val);
            };

            // Backwards check
            if (auto found = std::find_if(std::begin(RQ), rq_it, checker); found != rq_it) {
                //For debug
                // fmt::print("[RTQ][BACKWWARD] {} Duplicate RD Requests:  address: {} v_address: {} pf_metadata: {} current: {}\n", 
                //      __func__, 
                //      found->value().address.to<uint64_t>(),    // Convert address to uint64_t
                //      found->value().v_address.to<uint64_t>(),  // Convert v_address to uint64_t
                //      found->value().pf_metadata,               // Assuming pf_metadata is directly printable
                //      (current_time.time_since_epoch()) / clock_period // Convert to seconds since epoch
                // );
                auto instr_copy = std::move(found->value().instr_depend_on_me);
                auto ret_copy = std::move(found->value().to_return);

                std::set_union(std::begin(instr_copy), std::end(instr_copy),
                               std::begin(rq_it->value().instr_depend_on_me), std::end(rq_it->value().instr_depend_on_me), std::back_inserter(found->value().instr_depend_on_me));
                std::set_union(std::begin(ret_copy), std::end(ret_copy),
                               std::begin(rq_it->value().to_return), std::end(rq_it->value().to_return), std::back_inserter(found->value().to_return));

                rq_it->reset();
            }
            // Forwards check
            else if (auto found = std::find_if(std::next(rq_it), std::end(RQ), checker); found != std::end(RQ)) {
                //For debug
                // fmt::print("[RTQ][FORWARD] {} Duplicate RD Requests:  address: {} v_address: {} pf_metadata: {} current: {}\n", 
                //      __func__, 
                //      found->value().address.to<uint64_t>(),    // Convert address to uint64_t
                //      found->value().v_address.to<uint64_t>(),  // Convert v_address to uint64_t
                //      found->value().pf_metadata,               // Assuming pf_metadata is directly printable
                //      (current_time.time_since_epoch()) / clock_period // Convert to seconds since epoch
                // );

                auto instr_copy = std::move(found->value().instr_depend_on_me);
                auto ret_copy = std::move(found->value().to_return);

                std::set_union(std::begin(instr_copy), std::end(instr_copy),
                               std::begin(rq_it->value().instr_depend_on_me), std::end(rq_it->value().instr_depend_on_me), std::back_inserter(found->value().instr_depend_on_me));
                std::set_union(std::begin(ret_copy), std::end(ret_copy),
                               std::begin(rq_it->value().to_return), std::end(rq_it->value().to_return), std::back_inserter(found->value().to_return));

                rq_it->reset();
            }
            else {
                rq_it->value().forward_checked = true;
            }
        }
    }
}

void Ramulator2DRAM::process_response(uint64_t addr) {
    for (auto it = RQ.begin(); it != RQ.end(); ++it) {
        if (it->has_value() && it->value().address.to<uint64_t>() == addr) {
            auto& entry = it->value();

            response_type response{
                entry.address,
                entry.v_address,
                entry.data,
                entry.pf_metadata,
                entry.instr_depend_on_me
            };

            for (auto* ret : entry.to_return) {
                // fmt::print("[DEBUG] Sending response to LLC -> addr: {} v_addr: {} current: {}\n",
                    // response.address.to<uint64_t>(), response.v_address.to<uint64_t>(),
                    // (current_time.time_since_epoch()) / clock_period);
                ret->push_back(response);
            }

            // Remove from RQ since it's completed
            it->reset();
            return;
        }
    }
}

long Ramulator2DRAM::operate() {
    long progress = 0;
    // fmt::print("Ticking Check: \n");

    ramulator2_memorysystem->tick();
    initiate_requests();
    check_read_collision();

    return 1;
}

champsim::data::bytes Ramulator2DRAM::size() const {
    return champsim::data::bytes(total_dram_size);
}

void Ramulator2DRAM::finalize() {
    ramulator2_frontend->finalize();
    ramulator2_memorysystem->finalize();
}
