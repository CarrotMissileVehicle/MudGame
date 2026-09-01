/**
 * @file mining_calculator.h
 * @brief 采矿产量静态计算工具（MiningCalculator）。
 *
 * 根据经过时间与单次间隔，计算应产生的采矿次数及实际消耗时间。
 *
 * 依赖：time_service（time::gameDuration）。
 */
#pragma once

#include "time_service.h"

#include <cstddef>

using namespace mud;

/** @brief 采矿产量计算工具类（纯静态函数，无状态）。 */
class MiningCalculator
{
public:
    /** @brief 计算给定经过时间内应完成的采矿次数（仅需通过 interval 对齐）。 */
    static std::size_t calculate_production_count(
        time::gameDuration elapsed,
        time::gameDuration interval
    ) noexcept;

    /** @brief 计算实际消耗的时间（对齐到 interval 的整数倍）。 */
    static time::gameDuration calculate_consumed_time(
        time::gameDuration elapsed,
        time::gameDuration interval
    ) noexcept;
};