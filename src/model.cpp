#include "netsync/model.hpp"

#include <algorithm>
#include <stdexcept>

namespace netsync {

int apply_movement(int position, int movement) {
    if (position < kPositionMin || position > kPositionMax || movement < -1 || movement > 1) {
        throw std::invalid_argument("Position or movement is outside its supported range");
    }
    return std::clamp(position + movement, kPositionMin, kPositionMax);
}

Command Client::issue(int movement) {
    if (issued_ >= kMaxSequence) throw std::overflow_error("Maximum number of inputs reached");
    const int next = apply_movement(predicted_, movement);
    const Command command{issued_ + 1, movement};
    pending_.push_back(command);
    issued_ = command.sequence;
    predicted_ = next;
    return command;
}

std::vector<Command> Client::unacknowledged() const { return pending_; }

void Client::receive(const Snapshot& snapshot) {
    if (snapshot.acknowledged > issued_ ||
        snapshot.position < kPositionMin || snapshot.position > kPositionMax) {
        throw std::invalid_argument("Invalid server snapshot");
    }
    if (snapshot.acknowledged <= acknowledged_) return; // Ignore late/duplicate updates.

    const auto first = std::find_if(pending_.begin(), pending_.end(), [&](const Command& command) {
        return command.sequence > snapshot.acknowledged;
    });
    pending_.erase(pending_.begin(), first);
    acknowledged_ = snapshot.acknowledged;
    authoritative_ = snapshot.position;
    predicted_ = authoritative_;
    for (const Command& command : pending_) {
        predicted_ = apply_movement(predicted_, command.movement);
    }
}

void Server::receive(const std::vector<Command>& commands) {
    for (const Command& command : commands) {
        if (command.sequence == 0 || command.sequence > kMaxSequence ||
            command.movement < -1 || command.movement > 1) {
            throw std::invalid_argument("Invalid input command");
        }
        if (command.sequence <= acknowledged_) continue; // Idempotent retransmission.
        auto [existing, inserted] = buffered_.emplace(command.sequence, command.movement);
        if (!inserted && existing->second != command.movement) {
            throw std::invalid_argument("Conflicting command for an existing sequence");
        }
    }
    for (;;) {
        const auto entry = buffered_.find(acknowledged_ + 1);
        if (entry == buffered_.end()) break;
        position_ = apply_movement(position_, entry->second);
        buffered_.erase(entry);
        ++acknowledged_;
    }
}

} // namespace netsync
