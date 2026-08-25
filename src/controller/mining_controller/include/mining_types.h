#pragma once

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
}