#pragma once

#include "netsync/model.hpp"
#include "netsync/network.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace netsync {

// Retry the oldest unacknowledged inputs every N simulation ticks.
// Default settings preserve the original full-batch, every-tick behavior.
struct RetransmissionPolicy {
    std::size_t max_commands_per_packet = kMaxSequence;
    std::uint32_t resend_interval_ticks = 1;
};

struct Scenario {
    std::vector<int> movements;
    std::vector<Fault> upstream;
    std::vector<Fault> downstream;
    std::uint32_t drain_ticks = 64;
    RetransmissionPolicy retransmission{};
};

struct TickSample {
    std::uint64_t tick = 0;
    std::uint32_t issued = 0;
    std::uint32_t server_ack = 0;
    std::uint32_t client_ack = 0;
    int server_position = kStartingPosition;
    int predicted_position = kStartingPosition;
    std::size_t pending = 0;
    bool operator==(const TickSample&) const = default;
};

struct Result {
    std::vector<TickSample> timeline;
    LinkStatistics upstream;
    LinkStatistics downstream;
    std::size_t peak_state_gap = 0;
    std::size_t upstream_command_entries_sent = 0;
    std::size_t largest_upstream_batch = 0;
    bool converged = false;
};

Result simulate(const Scenario& scenario);
Scenario demo_scenario();
Scenario bounded_demo_scenario();
Scenario ideal_scenario();

} // namespace netsync
