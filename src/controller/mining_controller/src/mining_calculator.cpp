#include "mining_calculator.h"

// 根据已用时长与生产间隔，计算应产出的次数（向下取整）
std::size_t MiningCalculator::calculate_production_count(
    std::chrono::seconds elapsed,
    std::chrono::seconds interval
) noexcept
{
    // TODO: 实现
}

// 计算末次完整产出累计消耗的时长（用于记录剩余进度）
std::chrono::seconds MiningCalculator::calculate_consumed_time(
    std::chrono::seconds elapsed,
    std::chrono::seconds interval
) noexcept
{
    // TODO: 实现
}