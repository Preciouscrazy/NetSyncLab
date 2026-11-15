#pragma once

#include "netsync/model.hpp"
#include "netsync/network.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace netsync {

struct Scenario {
    std::vector<int> movements;
    std::vector<Fault> upstream;
    std::vector<Fault> downstream;
    std::uint32_t drain_ticks = 64;
};

struct TickSample {
    std::uint64_t tick = 0;
    std::uint32_t issued = 0;
    std::uint32_t server_ack = 0;
    std::uint32_t client_ack = 0;
    int server_position = kStartingPosition;
    int predicted_position = kStartingPosition;
    std::size_t pending = 0;
};

struct Result {
    std::vector<TickSample> timeline;
    LinkStatistics upstream;
    LinkStatistics downstream;
    std::size_t peak_state_gap = 0;
    bool converged = false;
};

Result simulate(const Scenario& scenario);
Scenario demo_scenario();
Scenario ideal_scenario();

} // namespace netsync
