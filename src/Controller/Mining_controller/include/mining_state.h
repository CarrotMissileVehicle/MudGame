/**
 * @file mining_state.h
 * @brief 采矿会话状态机（MiningState）。
 *
 * 维护一次采矿会话的状态：当前状态、目标层、开始时间与最近一次计时点。
 *
 * 依赖：mining_types、time_service。
 */
#pragma once

#include "mining_types.h"
#include "time_service.h"

#include <cstddef>
#include <optional>

using namespace mud;

/** @brief 采矿会话状态封装，供命令处理器与控制器读写共享状态。 */
class MiningState
{
private:
    mining::MiningStatus status_ = mining::MiningStatus::Idle; // 当前状态

    std::optional<std::size_t> layer_id_; // 采矿目标层（未采矿时为空）

    time::GameDateTime start_time_{}; // 本次采矿开始时间
    time::GameDateTime last_tick_{};  // 最近一次进度更新时间

public:
    /** @brief 是否处于采矿中。 */
    bool is_mining() const noexcept;

    /** @brief 返回当前采矿状态。 */
    mining::MiningStatus status() const noexcept;

    /** @brief 返回采矿目标层（空 optional 表示未在采矿）。 */
    const std::optional<std::size_t>& layer_id() const noexcept;

    /** @brief 返回本次采矿开始时间。 */
    time::GameDateTime start_time() const noexcept;

    /** @brief 返回最近一次进度更新时间。 */
    time::GameDateTime last_tick() const noexcept;

    /** @brief 开始采矿：设置状态、目标层与开始/计时时间。 */
    void start(
        std::size_t layer_id,
        const mud::time::GameDateTime& now
    );

    /** @brief 更新进度计时点为当前时间。 */
    void update_tick(
        const time::GameDateTime& time
    );

    /** @brief 停止采矿：复位状态为 Idle。 */
    void stop() noexcept;
};