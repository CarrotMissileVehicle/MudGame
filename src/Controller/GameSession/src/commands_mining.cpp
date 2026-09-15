/**
 * @file commands_mining.cpp
 * @brief 采矿指令注册（mine.start / mine.stop / mine.status）。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "WorldEngine.h"
#include "connector.h"

#include <string>

void GameCommands::register_mining(Connector& connector, GameContext& ctx, WorldEngine& world)
{
    // TUI 环境下以 MiningHandler 会话 + 后台时间线程 (tick_world) 非阻塞推进：
    // "开始采矿" 仅建立会话并提示，产出由后台逐游戏分钟结算，输入 q 或 mine.stop 结束。
    connector.bind("mine.start", [ctx, world](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtMine) {
            ctx.msg("你不在矿区。");
            return HandlerResult::Failed;
        }
        if (ctx.mining.is_mining()) {
            ctx.msg("当前已在采矿中。");
            return HandlerResult::Ok;
        }
        const std::size_t layer = static_cast<std::size_t>(ctx.opt_int(cmd, "layer", 0));
        const auto mc = world.build_mine_ctx();
        if (!ctx.mining_controller.can_enter(layer, mc)) {
            ctx.msg("层条件不满足（等级或照明不足）。");
            return HandlerResult::Failed;
        }
        if (!ctx.mining.start(layer, mc)) {
            ctx.msg("开始采矿失败。");
            return HandlerResult::Failed;
        }
        ctx.player.SetState(StateCode::Mining);
        ctx.msg("开始采矿（层 " + std::to_string(layer) + "）：产出随游戏时间自动结算，"
            "输入 q（结束动作）或 mine.stop 退出。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.stop", [ctx, world](const mud::cmd::Command&, const HandlerContext&) {
        if (!ctx.mining.is_mining()) {
            ctx.msg("当前并未在采矿。");
            return HandlerResult::Failed;
        }
        const auto results = ctx.mining.stop(world.build_mine_ctx());
        ctx.player.SetState(StateCode::Waiting);
        world.grant_mining(results);
        ctx.msg("采矿结束。");
        return HandlerResult::Ok;
    });

    connector.bind("mine.status", [ctx](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_mining_status(ctx.make_mining_view());
        return HandlerResult::Ok;
    });
}