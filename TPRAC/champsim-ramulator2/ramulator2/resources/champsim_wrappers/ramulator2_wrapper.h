#ifndef RAMULATOR2_WRAPPER_H
#define RAMULATOR2_WRAPPER_H

#include <array>
#include <string>
#include <limits>
#include <cmath>
#include <cstdint>  // for uint64_t, uint32_t, uint8_t
#include <deque>    // for deque
#include <optional>
#include "operable.h"
#include "chrono.h"
#include "channel.h"

#include <ramulator2/base/base.h>
#include <ramulator2/base/request.h>
#include <ramulator2/base/config.h>
#include <ramulator2/frontend/frontend.h>
#include <ramulator2/memory_system/memory_system.h>

//****************Wrapper of Ramulator2********************//
//We connect ChampSim and Ramulator2 using Response Queue structure (RQ and WQ)
//It triggers callback to LLC whenever the memory reqeusts are completed in ramulator2
class Ramulator2DRAM: public champsim::operable {
public:
    using response_type = typename champsim::channel::response_type;

    ~Ramulator2DRAM();

    // Packet in response queue
    struct request_type {
        uint8_t asid[2] = {std::numeric_limits<uint8_t>::max(), std::numeric_limits<uint8_t>::max()};
        uint32_t pf_metadata = 0;
        champsim::address address{};
        champsim::address v_address{};
        champsim::address data{};
        std::vector<uint64_t> instr_depend_on_me{};
        std::vector<std::deque<response_type>*> to_return{}; //Internal queue for callback

        explicit request_type(const typename champsim::channel::request_type& req)
            : address(req.address), v_address(req.v_address), data(req.data),
            instr_depend_on_me(req.instr_depend_on_me), to_return(req.to_return) {
                asid[0] = req.asid[0];
                asid[1] = req.asid[1];
            }
    };

    using queue_type = std::vector<std::optional<request_type>>;
    queue_type RQ; //READ Queue
    queue_type WQ; //WRITE Queue
  
    using channel_type = champsim::channel;
    std::vector<channel_type*> queues;

    bool add_rq(const request_type& packet, champsim::channel* ul);
    bool add_wq(const request_type& packet);

    long initiate_requests();
    void initialize();

    long operate() final;
    void finalize();
    void process_reponse(uint64_t addr);
    
    explicit Ramulator2DRAM(const std::string& config_path, std::vector<channel_type*>&& ul, std::size_t rq_size, std::size_t wq_size);

private:
    int num_mem_channels;
    std::unique_ptr<Ramulator::IFrontEnd> ramulator2_frontend;
    std::unique_ptr<Ramulator::IMemorySystem> ramulator2_memorysystem;
}

#endif // RAMULATOR2_WRAPPER_H
