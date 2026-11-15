#include "netsync/simulation.hpp"

#include <exception>
#include <iostream>
#include <string_view>

namespace {
void print_summary(const netsync::Result& result) {
    const auto& final = result.timeline.back();
    std::cout << "ticks=" << result.timeline.size() << '\n'
              << "issued_inputs=" << final.issued << '\n'
              << "server_ack=" << final.server_ack << '\n'
              << "client_ack=" << final.client_ack << '\n'
              << "server_position=" << final.server_position << '\n'
              << "predicted_position=" << final.predicted_position << '\n'
              << "peak_state_gap=" << result.peak_state_gap << '\n'
              << "upstream_sent=" << result.upstream.sent << '\n'
              << "upstream_dropped=" << result.upstream.dropped << '\n'
              << "upstream_reordered=" << result.upstream.reordered << '\n'
              << "downstream_sent=" << result.downstream.sent << '\n'
              << "downstream_dropped=" << result.downstream.dropped << '\n'
              << "downstream_reordered=" << result.downstream.reordered << '\n'
              << "converged=" << (result.converged ? "true" : "false") << '\n';
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: netsync demo | ideal | trace | compare | help\n";
        return 2;
    }
    const std::string_view command = argv[1];
    if (command == "help") {
        std::cout << "demo: scripted packet delay, loss and reordering\n"
                     "ideal: zero-delay, zero-loss reference\n"
                     "trace: CSV timeline of the demo scenario\n"
                     "compare: CSV full-batch vs bounded retransmission on identical faults\n";
        return 0;
    }
    if (command != "demo" && command != "ideal" && command != "trace" && command != "compare") {
        std::cerr << "Unknown command: " << command << '\n';
        return 2;
    }
    try {
        if (command == "compare") {
            const auto baseline = netsync::simulate(netsync::demo_scenario());
            const auto bounded = netsync::simulate(netsync::bounded_demo_scenario());
            const auto print_row = [](std::string_view label, const netsync::Result& r) {
                const auto& final = r.timeline.back();
                std::cout << label << ',' << r.timeline.size() << ','
                          << (r.converged ? "true" : "false") << ','
                          << final.server_ack << ',' << final.client_ack << ','
                          << r.upstream.sent << ',' << r.upstream.dropped << ','
                          << r.upstream_command_entries_sent << ','
                          << r.largest_upstream_batch << ',' << r.peak_state_gap << '\n';
            };
            std::cout << "policy,ticks,converged,server_ack,client_ack,upstream_packets_sent,"
                         "upstream_packets_dropped,command_entries_sent,largest_batch,peak_state_gap\n";
            print_row("baseline", baseline);
            print_row("bounded", bounded);
            return baseline.converged && bounded.converged ? 0 : 1;
        }
        const netsync::Result result = netsync::simulate(
            command == "ideal" ? netsync::ideal_scenario() : netsync::demo_scenario());
        if (command == "trace") {
            std::cout << "tick,issued,server_ack,client_ack,server_position,predicted_position,pending\n";
            for (const auto& s : result.timeline) {
                std::cout << s.tick << ',' << s.issued << ',' << s.server_ack << ','
                          << s.client_ack << ',' << s.server_position << ','
                          << s.predicted_position << ',' << s.pending << '\n';
            }
        } else {
            print_summary(result);
        }
        return result.converged ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 2;
    }
}
