/**
 * @file WorldEngine.h
 * @brief 世界推进引擎：驱动时间/天气/作物/钓鱼/集市联动与采矿结算。
 *
 * 由 1 秒后台节拍（TuiController::world_step）调用，职责取自原 main.cpp 的
 * tick_world / handle_action_tick / end_fishing / build_mine_ctx / grant_mining。
 */
#pragma once

#include <cstdint>
#include <vector>

#include "mining_types.h"

class GameContext;

/**
 * @brief 世界推进引擎：纯 Controller 编排，仅经由 GameContext 访问世界状态。
 */
class WorldEngine
{
public:
    explicit WorldEngine(GameContext& ctx);

    /// 每次试探性行动（钓鱼/采矿）等待的随机时间范围（毫秒）
    static constexpr int kActionWaitMinMs = 3000;
    static constexpr int kActionWaitMaxMs = 6000;
    /// 每次垂钓消耗的体力（饱食度）点数
    static constexpr int kFishingSatietyCost = 5;

    /// 世界推进：跨分钟驱动天气/作物/钓鱼/集市，并在采矿时按到期进度结算
    void tick_world();
    /// 1 秒后台节拍：非阻塞动作推进（当前仅钓鱼轮次）
    void handle_action_tick();
    /// 结束当前钓鱼动作（manual=true 表示手动退出）
    void end_fishing(bool manual);

    /// 由玩家采矿经验与矿镐等级推导采矿上下文
    mud::mining::MiningContext build_mine_ctx() const;
    /// 采矿产出入库：复制新 Object 进背包并累计采矿经验
    void grant_mining(const std::vector<mud::mining::MiningResult>& results, bool render = true);

private:
    GameContext& ctx_;
};