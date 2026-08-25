#pragma once

#include "time_service.h"

#include <cstddef>

using namespace mud;

class MiningCalculator
{
public:
    static std::size_t calculate_production_count(
        time::gameDuration elapsed,
        time::gameDuration interval
    ) noexcept;

    static time::gameDuration calculate_consumed_time(
        time::gameDuration elapsed,
        time::gameDuration interval
    ) noexcept;
};