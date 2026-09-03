/**
 * @file mining_handler.cpp
 * @brief 采矿命令处理器实现。
 *
 * 将命令层调用转发给采矿控制器，并维护一个会话状态实例。
 */
#include "mining_handler.h"

MiningHandler::MiningHandler(MiningController& controller)
    : controller_(controller)
{
}

bool MiningHandler::start(
    const std::size_t layer_id,
    const mining::MiningContext& context)
{
    return controller_.start_mining(state_, layer_id, context);
}

std::vector<mining::MiningResult> MiningHandler::poll(
    const mining::MiningContext& context)
{
    return controller_.update(state_, context);
}

std::vector<mining::MiningResult> MiningHandler::stop(
    const mining::MiningContext& context)
{
    return controller_.stop_mining(state_, context);
}

bool MiningHandler::is_mining() const noexcept
{
    return controller_.is_mining(state_);
}

mining::MiningStatus MiningHandler::status() const noexcept
{
    return state_.status();
}