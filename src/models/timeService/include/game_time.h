/**
 * @file game_time.h
 * @brief 游戏时间基础类型别名定义。
 *
 * 统一游戏的时间点与时长类型，避免各处直接依赖 std::chrono。
 */
#pragma once

#include <chrono>

namespace mud::time
{

    using gameTimePoint = std::chrono::steady_clock::time_point; // 游戏时间点
    using gameDuration = std::chrono::milliseconds;              // 游戏时间步长（毫秒）

}