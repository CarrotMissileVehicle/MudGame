#pragma once

#include <chrono>
#include <cstddef>

class MiningCalculator
{
public:
    static std::size_t calculate_production_count(
        std::chrono::seconds elapsed,
        std::chrono::seconds interval
    ) noexcept;

    static std::chrono::seconds calculate_consumed_time(
        std::chrono::seconds elapsed,
        std::chrono::seconds interval
    ) noexcept;
};