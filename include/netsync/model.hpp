#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace netsync {

inline constexpr int kPositionMin = 0;
inline constexpr int kPositionMax = 20;
inline constexpr int kStartingPosition = 10;
inline constexpr std::uint32_t kMaxSequence = 100000;

struct Command {
    std::uint32_t sequence = 0;
    int movement = 0;
    bool operator==(const Command&) const = default;
};

struct Snapshot {
    std::uint32_t acknowledged = 0;
    int position = 0;
    bool operator==(const Snapshot&) const = default;
};

int apply_movement(int position, int movement);

class Client {
public:
    Command issue(int movement);
    std::vector<Command> unacknowledged() const;
    void receive(const Snapshot& snapshot);

    [[nodiscard]] int predicted_position() const noexcept { return predicted_; }
    [[nodiscard]] int authoritative_position() const noexcept { return authoritative_; }
    [[nodiscard]] std::uint32_t last_issued() const noexcept { return issued_; }
    [[nodiscard]] std::uint32_t last_acknowledged() const noexcept { return acknowledged_; }
    [[nodiscard]] std::size_t pending_count() const noexcept { return pending_.size(); }

private:
    std::uint32_t issued_ = 0;
    std::uint32_t acknowledged_ = 0;
    int authoritative_ = kStartingPosition;
    int predicted_ = kStartingPosition;
    std::vector<Command> pending_;
};

class Server {
public:
    void receive(const std::vector<Command>& commands);
    [[nodiscard]] Snapshot snapshot() const noexcept { return {acknowledged_, position_}; }
    [[nodiscard]] std::size_t buffered_count() const noexcept { return buffered_.size(); }

private:
    std::uint32_t acknowledged_ = 0;
    int position_ = kStartingPosition;
    std::map<std::uint32_t, int> buffered_;
};

} // namespace netsync
