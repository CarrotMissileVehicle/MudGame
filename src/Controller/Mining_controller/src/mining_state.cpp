/**
 * @file mining_state.cpp
 * @brief 采矿会话状态机实现。
 */
#include "mining_state.h"

// 是否处于采矿状态
bool MiningState::is_mining() const noexcept
{
    return status_ == mining::MiningStatus::Mining;
}

// 返回当前状态
mining::MiningStatus MiningState::status() const noexcept
{
    return status_;
}

// 返回当前采矿目标层（空表示未采矿）
const std::optional<std::size_t>& MiningState::layer_id() const noexcept
{
    return layer_id_;
}

// 返回本次采矿开始时间点
time::GameDateTime MiningState::start_time() const noexcept
{
    return start_time_;
}

// 返回最近一次心跳时间点
time::GameDateTime MiningState::last_tick() const noexcept
{
    return last_tick_;
}

// 以指定层与时间点启动采矿状态
void MiningState::start(
    std::size_t layer_id,
    time::GameDateTime now
)
{
    layer_id_ = layer_id;
    status_ = mining::MiningStatus::Mining;

    start_time_ = now;
    last_tick_ = now;
}

// 刷新心跳时间点
void MiningState::update_tick(
    time::GameDateTime time
)
{
    last_tick_ = time;
}

// 停止采矿并清空相关状态
void MiningState::stop() noexcept
{
    status_ = mining::MiningStatus::Idle;
    layer_id_.reset();
    start_time_ = {};
    last_tick_ = {};
}