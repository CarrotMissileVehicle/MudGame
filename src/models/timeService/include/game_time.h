#pragma once

#include <chrono>

namespace mud::time
{

    using gameTimePoint = std::chrono::system_clock::time_point;
    using gameDuration = std::chrono::milliseconds;

}