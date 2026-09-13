/**
 * @file commands_misc.cpp
 * @brief 杂项指令注册：时间 / 天气 / 玩家 / 移动 / 钓鱼 / 存档读档。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "WorldEngine.h"
#include "connector.h"
#include "PlayerSerializer.h"

#include <cstdint>
#include <string>

namespace
{
    // 默认存档文件名
    constexpr const char* kSaveFileName = "mudgame.sav";
}

void GameCommands::register_misc(Connector& connector, GameContext& ctx, WorldEngine& world)
{
    // 时间
    connector.bind("time.now", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.render_now();
        return HandlerResult::Ok;
    });

    connector.bind("time.scale", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        const double factor = std::stod(cmd.options.at("factor"));
        ctx.time.set_time_scale(factor);
        ctx.view.render_time_scale(factor);
        return HandlerResult::Ok;
    });

    // 玩家状态 / 移动
    connector.bind("player.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_status(ctx.make_player_status());
        return HandlerResult::Ok;
    });

    const auto bind_move = [&](const std::string& verb, bool (Move::*fn)()) {
        connector.bind(verb, [&, fn](const mud::cmd::Command&, const HandlerContext&) {
            if (!ctx.weather.can_go_outside()) {
                ctx.msg(ctx.weather.weather_name() + "天不宜外出。");
                return HandlerResult::Failed;
            }
            ctx.player.SetState(StateCode::Moving);
            if ((ctx.move.*fn)()) {
                ctx.msg("移动成功。");
                ctx.view.render_map(ctx.player.GetPosition());
            } else {
                ctx.msg("这个方向走不通。");
            }
            ctx.player.SetState(StateCode::Waiting); // 一次性动作完成后复位，避免状态粘滞
            return HandlerResult::Ok;
        });
    };
    bind_move("move.up",    &Move::GoUp);
    bind_move("move.down",  &Move::GoDown);
    bind_move("move.left",  &Move::GoLeft);
    bind_move("move.right", &Move::GoRight);

    // 钓鱼（TUI 非阻塞版）：启动后由 handle_action_tick 按 3-6 秒节奏推进，
    // 输入 q 或再次 fish.tick（动作进行中）可结束。可反复 start/stop。
    connector.bind("fish.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_fishing(ctx.make_fishing_view());
        return HandlerResult::Ok;
    });

    connector.bind("fish.tick", [&](const mud::cmd::Command&, const HandlerContext&) {
        if (ctx.tui.action_type == mud::tui::ActionType::Fish) {
            world.end_fishing(true);
            return HandlerResult::Ok;
        }
        if (ctx.player.GetPosition() != AtCoast) {
            ctx.msg("你不在海边。");
            return HandlerResult::Failed;
        }
        if (!ctx.weather.can_fish()) {
            ctx.msg(ctx.weather.weather_name() + "天不能钓鱼。");
            return HandlerResult::Failed;
        }
        if (ctx.player.GetSatiety() <= 0) {
            ctx.msg("体力不足，无法钓鱼。");
            return HandlerResult::Failed;
        }
        const auto now = std::chrono::steady_clock::now();
        ctx.tui.action_type = mud::tui::ActionType::Fish;
        ctx.tui.action_cycles = 0;
        ctx.tui.action_next_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
            + WorldEngine::kActionWaitMinMs + std::rand() % (WorldEngine::kActionWaitMaxMs - WorldEngine::kActionWaitMinMs + 1);
        ctx.player.SetState(StateCode::Fishing);
        ctx.msg("开始钓鱼：每 3-6 秒自动出结果，输入 q（结束动作）或 fish.tick 退出。");
        return HandlerResult::Ok;
    });

    // 天气
    connector.bind("weather.now", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_weather(ctx.make_weather_view());
        return HandlerResult::Ok;
    });

    // 存档 / 读档
    connector.bind("save", [&](const mud::cmd::Command&, const HandlerContext&) {
        PlayerSerializer serializer;
        if (serializer.Save(kSaveFileName, ctx.player, ctx.game, ctx.gold, ctx.tools,
                            ctx.time.session_total())) {
            ctx.msg(std::string("存档成功：") + kSaveFileName);
        } else {
            ctx.msg("存档失败（无法写入存档文件）。");
        }
        return HandlerResult::Ok;
    });

    connector.bind("load", [&](const mud::cmd::Command&, const HandlerContext&) {
        PlayerSerializer serializer;
        std::int64_t totalMinutes = -1;
        if (!serializer.Load(kSaveFileName, ctx.player, ctx.game, ctx.gold, ctx.tools, totalMinutes)) {
            ctx.msg(std::string("读档失败：没有找到存档 ") + kSaveFileName + "。");
            return HandlerResult::Failed;
        }
        if (totalMinutes >= 0) {
            mud::time::GameDateTime t;
            t.advance(totalMinutes); // 由存档总分钟数重建游戏内时钟
            ctx.time.set_time(t);
        }
        ctx.msg(std::string("读档成功，继续你的冒险吧。"));
        return HandlerResult::Ok;
    });
}