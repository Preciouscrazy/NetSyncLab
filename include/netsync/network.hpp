#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace netsync {

struct Fault {
    std::uint32_t delay_ticks = 0;
    bool drop = false;
};

struct LinkStatistics {
    std::size_t sent = 0;
    std::size_t dropped = 0;
    std::size_t delivered = 0;
    std::size_t reordered = 0;
};

// A deliberately bounded, in-memory link. Pattern indices refer to send order.
// This models entire dropped/delayed packets, not a real network transport.
template <typename Payload>
class PacketLink {
public:
    explicit PacketLink(std::vector<Fault> pattern) : pattern_(std::move(pattern)) {
        if (pattern_.empty()) throw std::invalid_argument("Fault pattern cannot be empty");
        for (const Fault& fault : pattern_) {
            if (fault.delay_ticks > 1000) throw std::invalid_argument("Network delay exceeds 1000 ticks");
        }
    }

    void send(std::uint64_t tick, Payload payload) {
        const std::size_t ordinal = stats_.sent++;
        const Fault fault = pattern_[ordinal % pattern_.size()];
        if (fault.drop) {
            ++stats_.dropped;
            return;
        }
        queued_.push_back({ordinal, tick + fault.delay_ticks, std::move(payload)});
    }

    std::vector<Payload> deliver(std::uint64_t tick) {
        std::vector<Packet> ready;
        std::vector<Packet> waiting;
        ready.reserve(queued_.size());
        waiting.reserve(queued_.size());
        for (Packet& packet : queued_) {
            if (packet.deliver_at <= tick) ready.push_back(std::move(packet));
            else waiting.push_back(std::move(packet));
        }
        queued_.swap(waiting);
        std::sort(ready.begin(), ready.end(), [](const Packet& a, const Packet& b) {
            if (a.deliver_at != b.deliver_at) return a.deliver_at < b.deliver_at;
            return a.ordinal < b.ordinal;
        });
        std::vector<Payload> delivered;
        delivered.reserve(ready.size());
        for (Packet& packet : ready) {
            if (has_delivered_ && packet.ordinal < highest_delivered_) ++stats_.reordered;
            if (!has_delivered_ || packet.ordinal > highest_delivered_) {
                highest_delivered_ = packet.ordinal;
                has_delivered_ = true;
            }
            ++stats_.delivered;
            delivered.push_back(std::move(packet.payload));
        }
        return delivered;
    }

    [[nodiscard]] LinkStatistics statistics() const noexcept { return stats_; }
    [[nodiscard]] std::size_t in_flight() const noexcept { return queued_.size(); }

private:
    struct Packet {
        std::size_t ordinal;
        std::uint64_t deliver_at;
        Payload payload;
    };
    std::vector<Fault> pattern_;
    std::vector<Packet> queued_;
    LinkStatistics stats_;
    std::size_t highest_delivered_ = 0;
    bool has_delivered_ = false;
};

} // namespace netsync
