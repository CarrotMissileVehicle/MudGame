#pragma once

#include "mining_types.h"

#include <chrono>
#include <optional>

namespace mud::mining
{
    struct MiningState
    {
        MiningStatus status = MiningStatus::Idle;

        std::optional<size_t> layer_id;

        std::chrono::steady_clock::time_point start_time{};
        std::chrono::steady_clock::time_point last_tick{};

        size_t produced_count = 0;
    };
}