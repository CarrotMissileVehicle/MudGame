#pragma once

#include "mining_types.h"

#include <chrono>
#include <cstddef>
#include <optional>

using namespace mud::mining;

class MiningState
{
private:
    MiningStatus status_ = MiningStatus::Idle;

    std::optional<std::size_t> layer_id_;

    std::chrono::steady_clock::time_point start_time_{};
    std::chrono::steady_clock::time_point last_tick_{};

public:
    bool is_mining() const noexcept;

    MiningStatus status() const noexcept;

    const std::optional<std::size_t>& layer_id() const noexcept;

    std::chrono::steady_clock::time_point start_time() const noexcept;

    std::chrono::steady_clock::time_point last_tick() const noexcept;

    void start(
        std::size_t layer_id,
        std::chrono::steady_clock::time_point now
    );

    void update_tick(
        std::chrono::steady_clock::time_point time
    );

    void stop() noexcept;
};