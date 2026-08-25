#pragma once

#include <chrono>
#include <cstddef>
#include <string>

namespace mud::mining
{
    enum class MiningStatus
    {
        Idle,
        Mining
    };

    enum class MiningLayer
    {
        Shallow,
        Middle,
        Deep,
        Crystal,
        Core
    };

    enum class MiningSpeed
    {
        Core = 1,
        Crystal = 3,
        Gold = 5,
        Silver = 9,
        Iron = 13,
    };

    struct MiningTool // 等工具部分完善
    {
        MiningSpeed mining_speed;
        std::chrono::seconds interval{static_cast<size_t>(mining_speed)};
        double rare_bonus = 0.0;
    };

    struct MiningContext
    {
        std::size_t mining_level = 1;

        bool has_torch = false;
        bool has_lantern = false;

        MiningTool tool{};
    };

    struct MiningResult
    {
        std::string ore_id;
        std::size_t experience = 0;
        std::size_t quantity = 1;
    };
}