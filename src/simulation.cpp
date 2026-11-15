#include "netsync/simulation.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace netsync {

Result simulate(const Scenario& scenario) {
    if (scenario.movements.empty() || scenario.movements.size() > kMaxSequence ||
        scenario.drain_ticks > 10000) {
        throw std::invalid_argument("Scenario needs 1..100000 inputs and at most 10000 drain ticks");
    }

    Client client;
    Server server;
    PacketLink<std::vector<Command>> upstream(scenario.upstream);
    PacketLink<Snapshot> downstream(scenario.downstream);
    Result result;

    const std::uint64_t limit = scenario.movements.size() + scenario.drain_ticks;
    for (std::uint64_t tick = 0; tick < limit; ++tick) {
        if (tick < scenario.movements.size()) client.issue(scenario.movements[static_cast<std::size_t>(tick)]);
        // Carry all unacknowledged inputs, so a dropped packet need not lose its commands.
        if (client.pending_count() > 0) upstream.send(tick, client.unacknowledged());
        for (const auto& batch : upstream.deliver(tick)) server.receive(batch);

        // Acknowledgment snapshots are independent packets and can be lost or reordered.
        downstream.send(tick, server.snapshot());
        for (const Snapshot& snapshot : downstream.deliver(tick)) client.receive(snapshot);

        const Snapshot authoritative = server.snapshot();
        const int gap = std::abs(client.predicted_position() - authoritative.position);
        result.peak_state_gap = std::max(result.peak_state_gap, static_cast<std::size_t>(gap));
        result.timeline.push_back({tick, client.last_issued(), authoritative.acknowledged,
                                   client.last_acknowledged(), authoritative.position,
                                   client.predicted_position(), client.pending_count()});

        if (client.pending_count() == 0 && client.last_issued() == authoritative.acknowledged &&
            tick + 1 >= scenario.movements.size()) {
            result.converged = client.predicted_position() == authoritative.position;
            break;
        }
    }
    result.upstream = upstream.statistics();
    result.downstream = downstream.statistics();
    return result;
}

Scenario ideal_scenario() {
    return {{1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1}, {{0, false}}, {{0, false}}, 16};
}

Scenario demo_scenario() {
    auto scenario = ideal_scenario();
    scenario.upstream = {{2, false}, {0, false}, {1, true}, {3, false}, {0, false}};
    scenario.downstream = {{1, false}, {2, true}, {0, false}, {3, false}};
    scenario.drain_ticks = 64;
    return scenario;
}

} // namespace netsync
