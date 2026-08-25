#pragma once

#include <chrono>

namespace mud::mining
{
    class MiningCalculator
    {
    public:
        static size_t calculateProductionCount(
            std::chrono::seconds elapsed,
            std::chrono::seconds interval
        );

        static std::chrono::seconds calculateRemainingTime(
            std::chrono::seconds elapsed,
            std::chrono::seconds interval
        );
    };
}