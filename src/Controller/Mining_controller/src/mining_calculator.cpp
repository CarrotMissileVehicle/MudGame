/**
 * @file mining_calculator.cpp
 * @brief 采矿产量计算工具实现。
 */
#include "mining_calculator.h"

// 根据已用时长与生产间隔，计算应产出的次数（向下取整）
std::size_t MiningCalculator::calculate_production_count(
    const time::gameMinutes elapsed,
    const time::gameMinutes interval
) noexcept
{
    if (elapsed <= 0 || interval <= 0)
        return 0;
    return static_cast<std::size_t>(elapsed / interval);
}

// 计算末次完整产出累计消耗的时长（用于记录剩余进度）
time::gameMinutes MiningCalculator::calculate_consumed_time(
    const time::gameMinutes elapsed,
    const time::gameMinutes interval
) noexcept
{
    if (elapsed <= 0 || interval <= 0)
        return 0;
    return (elapsed / interval) * interval;
}