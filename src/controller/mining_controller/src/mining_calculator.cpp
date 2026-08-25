#include "mining_calculator.h"

// 根据已用时长与生产间隔，计算应产出的次数（向下取整）
std::size_t MiningCalculator::calculate_production_count(
    time::gameDuration elapsed,
    time::gameDuration interval
) noexcept
{
    // TODO: 实现
}

// 计算末次完整产出累计消耗的时长（用于记录剩余进度）
time::gameDuration MiningCalculator::calculate_consumed_time(
    time::gameDuration elapsed,
    time::gameDuration interval
) noexcept
{
    // TODO: 实现
}