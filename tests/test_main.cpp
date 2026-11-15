#include "netsync/model.hpp"
#include "netsync/network.hpp"
#include "netsync/simulation.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace netsync;

namespace {
void require(bool condition, const std::string& label) {
    if (!condition) throw std::runtime_error(label);
}

template <typename Fn>
void invalid(Fn&& action) {
    try { action(); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Expected invalid_argument");
}

void movement_clamps_to_grid() {
    require(apply_movement(0, -1) == 0, "minimum");
    require(apply_movement(20, 1) == 20, "maximum");
    require(apply_movement(10, -1) == 9, "left");
    require(apply_movement(10, 0) == 10, "idle");
}

void movement_rejects_invalid_values() {
    invalid([] { (void)apply_movement(-1, 1); });
    invalid([] { (void)apply_movement(21, -1); });
    invalid([] { (void)apply_movement(10, 2); });
}

void client_predicts_immediately() {
    Client client;
    require(client.issue(1) == Command{1, 1}, "sequence one");
    require(client.issue(-1) == Command{2, -1}, "sequence two");
    require(client.predicted_position() == 10 && client.pending_count() == 2, "predictions");
}

void client_reconciles_unconfirmed_inputs() {
    Client client;
    client.issue(1);
    client.issue(1);
    client.issue(-1);
    client.receive({1, 11});
    require(client.last_acknowledged() == 1, "acknowledged input");
    require(client.pending_count() == 2, "remaining inputs");
    require(client.predicted_position() == 11, "replayed remaining inputs");
    client.receive({3, 11});
    require(client.pending_count() == 0 && client.predicted_position() == 11, "final state");
}

void client_corrects_authoritative_position() {
    Client client;
    client.issue(1);
    client.issue(1);
    client.issue(-1);
    // A different authoritative baseline forces a correction, then two inputs replay.
    client.receive({1, 9});
    require(client.authoritative_position() == 9, "server baseline not applied");
    require(client.predicted_position() == 9, "outstanding inputs not replayed after correction");
    require(client.pending_count() == 2, "pending inputs lost");
}

void client_ignores_delayed_snapshot() {
    Client client;
    client.issue(1);
    client.issue(1);
    client.receive({2, 12});
    client.receive({1, 11});
    client.receive({2, 5});
    require(client.predicted_position() == 12 && client.last_acknowledged() == 2, "stale snapshot changed state");
}

void client_rejects_invalid_snapshots() {
    Client client;
    client.issue(1);
    invalid([&] { client.receive({2, 10}); });
    invalid([&] { client.receive({1, -1}); });
    invalid([&] { client.receive({0, 21}); });
}

void client_rejects_invalid_input_without_mutation() {
    Client client;
    invalid([&] { client.issue(8); });
    require(client.last_issued() == 0 && client.pending_count() == 0, "invalid issue mutated client");
}

void server_applies_commands_in_order() {
    Server server;
    server.receive({{1, 1}, {2, -1}, {3, 1}});
    require(server.snapshot() == Snapshot{3, 11}, "authoritative state");
}

void server_buffers_out_of_order_commands() {
    Server server;
    server.receive({{2, -1}, {3, 1}});
    require(server.snapshot() == Snapshot{0, 10} && server.buffered_count() == 2, "gap should block processing");
    server.receive({{1, 1}});
    require(server.snapshot() == Snapshot{3, 11} && server.buffered_count() == 0, "gap closed");
}

void server_ignores_retransmitted_commands() {
    Server server;
    server.receive({{1, 1}});
    server.receive({{1, 1}, {2, 1}});
    require(server.snapshot() == Snapshot{2, 12}, "duplicate command was applied twice");
}

void server_rejects_conflicting_buffered_command() {
    Server server;
    server.receive({{3, 1}});
    invalid([&] { server.receive({{3, -1}}); });
    require(server.buffered_count() == 1, "conflict mutated buffered command");
}

void server_rejects_invalid_sequence_and_movement() {
    Server server;
    invalid([&] { server.receive({{0, 1}}); });
    invalid([&] { server.receive({{kMaxSequence + 1, 1}}); });
    invalid([&] { server.receive({{1, 2}}); });
}

void packet_link_delays_delivery() {
    PacketLink<int> link({{2, false}});
    link.send(0, 17);
    require(link.deliver(0).empty() && link.deliver(1).empty(), "early delivery");
    require(link.deliver(2) == std::vector<int>{17}, "late delivery");
    require(link.statistics().delivered == 1, "delivered count");
}

void packet_link_tracks_drops() {
    PacketLink<int> link({{0, true}, {0, false}});
    link.send(0, 3);
    link.send(0, 4);
    require(link.deliver(0) == std::vector<int>{4}, "drop pattern");
    require(link.statistics().sent == 2 && link.statistics().dropped == 1, "drop metrics");
}

void packet_link_tracks_reordering() {
    PacketLink<int> link({{3, false}, {0, false}});
    link.send(0, 10);
    link.send(1, 20);
    require(link.deliver(1) == std::vector<int>{20}, "second arrived first");
    require(link.deliver(3) == std::vector<int>{10}, "first arrived later");
    require(link.statistics().reordered == 1, "reorder metric");
}

void packet_link_rejects_bad_pattern() {
    invalid([] { (void)PacketLink<int>({}); });
    invalid([] { (void)PacketLink<int>({{1001, false}}); });
}

void ideal_scenario_converges_without_gap() {
    const Result result = simulate(ideal_scenario());
    require(result.converged, "ideal scenario did not converge");
    require(result.peak_state_gap == 0, "immediate link should not have a state gap");
    require(result.timeline.size() == ideal_scenario().movements.size(), "unexpected ticks");
    require(result.upstream.dropped == 0 && result.downstream.dropped == 0, "unexpected drops");
}

void demo_scenario_converges_after_packet_loss() {
    const Result result = simulate(demo_scenario());
    require(result.converged, "demo should converge");
    require(result.upstream.dropped > 0 && result.downstream.dropped > 0, "demo must include loss");
    require(result.upstream.reordered > 0, "demo must include reordering");
    const auto& final = result.timeline.back();
    require(final.issued == 12 && final.server_ack == 12 && final.client_ack == 12, "acks not converged");
    require(final.predicted_position == final.server_position, "position not converged");
}

void scripted_scenario_is_repeatable() {
    const auto first = simulate(demo_scenario());
    const auto second = simulate(demo_scenario());
    require(first.timeline.size() == second.timeline.size(), "tick count changed");
    require(first.peak_state_gap == second.peak_state_gap, "gap changed");
    require(first.upstream.dropped == second.upstream.dropped, "drop count changed");
    for (std::size_t i = 0; i < first.timeline.size(); ++i) {
        const auto& a = first.timeline[i];
        const auto& b = second.timeline[i];
        require(a.server_ack == b.server_ack && a.client_ack == b.client_ack &&
                a.predicted_position == b.predicted_position && a.server_position == b.server_position,
                "scenario trace changed");
    }
}

void completely_lost_upstream_cannot_converge() {
    Scenario scenario{{1, 1}, {{0, true}}, {{0, false}}, 5};
    const auto result = simulate(scenario);
    require(!result.converged && result.timeline.size() == 7, "unrecoverable loss should time out");
    require(result.timeline.back().server_ack == 0, "server should never apply an input");
}

void completely_lost_downstream_cannot_converge() {
    Scenario scenario{{1, 1}, {{0, false}}, {{0, true}}, 5};
    const auto result = simulate(scenario);
    require(!result.converged && result.timeline.back().server_ack == 2, "server needs to receive inputs");
    require(result.timeline.back().client_ack == 0, "client must not fabricate an ack");
}

void simulation_rejects_invalid_scenarios() {
    invalid([] { (void)simulate({{}, {{0, false}}, {{0, false}}, 1}); });
    invalid([] { (void)simulate({{2}, {{0, false}}, {{0, false}}, 1}); });
    invalid([] { (void)simulate({{1}, {}, {{0, false}}, 1}); });
    invalid([] { (void)simulate({{1}, {{0, false}}, {{0, false}}, 10001}); });
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> checks{
        {"movement_clamps_to_grid", movement_clamps_to_grid},
        {"movement_rejects_invalid_values", movement_rejects_invalid_values},
        {"client_predicts_immediately", client_predicts_immediately},
        {"client_reconciles_unconfirmed_inputs", client_reconciles_unconfirmed_inputs},
        {"client_corrects_authoritative_position", client_corrects_authoritative_position},
        {"client_ignores_delayed_snapshot", client_ignores_delayed_snapshot},
        {"client_rejects_invalid_snapshots", client_rejects_invalid_snapshots},
        {"client_rejects_invalid_input_without_mutation", client_rejects_invalid_input_without_mutation},
        {"server_applies_commands_in_order", server_applies_commands_in_order},
        {"server_buffers_out_of_order_commands", server_buffers_out_of_order_commands},
        {"server_ignores_retransmitted_commands", server_ignores_retransmitted_commands},
        {"server_rejects_conflicting_buffered_command", server_rejects_conflicting_buffered_command},
        {"server_rejects_invalid_sequence_and_movement", server_rejects_invalid_sequence_and_movement},
        {"packet_link_delays_delivery", packet_link_delays_delivery},
        {"packet_link_tracks_drops", packet_link_tracks_drops},
        {"packet_link_tracks_reordering", packet_link_tracks_reordering},
        {"packet_link_rejects_bad_pattern", packet_link_rejects_bad_pattern},
        {"ideal_scenario_converges_without_gap", ideal_scenario_converges_without_gap},
        {"demo_scenario_converges_after_packet_loss", demo_scenario_converges_after_packet_loss},
        {"scripted_scenario_is_repeatable", scripted_scenario_is_repeatable},
        {"completely_lost_upstream_cannot_converge", completely_lost_upstream_cannot_converge},
        {"completely_lost_downstream_cannot_converge", completely_lost_downstream_cannot_converge},
        {"simulation_rejects_invalid_scenarios", simulation_rejects_invalid_scenarios},
    };
    int failures = 0;
    for (const auto& [name, check] : checks) {
        try {
            check();
            std::cout << "PASS " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
        }
    }
    std::cout << "RESULT " << checks.size() - failures << '/' << checks.size() << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
