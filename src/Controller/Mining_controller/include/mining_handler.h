/**
 * @file mining_handler.h
 * @brief 采矿命令处理器。
 *
 * 编排采矿控制器（MiningController）与会话状态（MiningState），
 * 向命令层提供 start / poll / stop 等一次性命令入口。
 */
#pragma once

#include "mining_controller.h"

#include <optional>

/** @brief 采矿命令处理器：持有一个会话状态并转发给采矿控制器。 */
class MiningHandler
{
public:
    explicit MiningHandler(MiningController& controller);

    /** @brief 在目标层启动一次采矿会话。 */
    bool start(
        std::size_t layer_id,
        const mining::MiningContext& context);

    /** @brief 推进进度，返回本次到期的产出。 */
    std::vector<mining::MiningResult> poll(
        const mining::MiningContext& context);

    /** @brief 停止采矿并结算未提取产出。 */
    std::vector<mining::MiningResult> stop(
        const mining::MiningContext& context);

    /** @brief 当前会话是否处于采矿中。 */
    bool is_mining() const noexcept;

    /** @brief 当前会话状态。 */
    mining::MiningStatus status() const noexcept;

    /** @brief 当前会话目标层（未采矿时为空）。 */
    const std::optional<std::size_t>& layer_id() const noexcept;

    /** @brief 本次采矿开始时间。 */
    time::GameDateTime start_time() const noexcept;

private:
    MiningController& controller_; // 采矿控制器（外部所有）
    MiningState state_;            // 本处理器持有的会话状态
};