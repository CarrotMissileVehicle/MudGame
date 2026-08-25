#include "mining_state.h"

// 是否处于采矿状态
bool MiningState::is_mining() const noexcept
{
    // TODO: 实现
}

// 返回当前状态
mining::MiningStatus MiningState::status() const noexcept
{
    // TODO: 实现
}

// 返回当前所在矿区 id（空表示未采矿）
const std::optional<std::size_t>& MiningState::layer_id() const noexcept
{
    // TODO: 实现
}

// 返回本次采矿开始时间点
time::gameTimePoint MiningState::start_time() const noexcept
{
    // TODO: 实现
}

// 返回最近一次心跳时间点
time::gameTimePoint MiningState::last_tick() const noexcept
{
    // TODO: 实现
}

// 以指定层与时间点启动采矿状态
void MiningState::start(
    std::size_t layer_id,
    time::gameTimePoint now
)
{
    // TODO: 实现
}

// 刷新心跳时间点
void MiningState::update_tick(
    time::gameTimePoint time
)
{
    // TODO: 实现
}

// 停止采矿并清空相关状态
void MiningState::stop() noexcept
{
    // TODO: 实现
}