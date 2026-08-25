#pragma once

#include <chrono>

namespace mud {

    using GameTimePoint = std::chrono::system_clock::time_point;
    using GameDuration = std::chrono::milliseconds;

}