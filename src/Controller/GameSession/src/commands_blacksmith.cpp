/**
 * @file commands_blacksmith.cpp
 * @brief 工具与铁匠铺指令注册（tools.status / blacksmith.status / blacksmith.repair）。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "connector.h"

#include <string>

void GameCommands::register_blacksmith(Connector& connector, GameContext& ctx)
{
    connector.bind("tools.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        ctx.view.render_tools(ctx.make_tools_view());
        return HandlerResult::Ok;
    });

    // 铁匠铺（集市内，工具修复服务）
    connector.bind("blacksmith.status", [&](const mud::cmd::Command&, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇，先去小镇集市看看。");
            return HandlerResult::Failed;
        }
        ctx.view.render_blacksmith(ctx.make_blacksmith_view());
        return HandlerResult::Ok;
    });

    connector.bind("blacksmith.repair", [&](const mud::cmd::Command& cmd, const HandlerContext&) {
        if (ctx.player.GetPosition() != AtTown) {
            ctx.msg("你不在城镇，无法前往铁匠铺。");
            return HandlerResult::Failed;
        }
        const std::string tool_key = cmd.options.count("tool") ? cmd.options.at("tool") : "";
        const std::string method = cmd.options.count("method") ? cmd.options.at("method") : "";
        mud::tool::ToolId id;
        if (tool_key == "hoe")             id = mud::tool::ToolId::Hoe;
        else if (tool_key == "rod")        id = mud::tool::ToolId::Rod;
        else if (tool_key == "pickaxe")    id = mud::tool::ToolId::Pickaxe;
        else {
            ctx.msg("未知工具：" + tool_key + "（可用 hoe/rod/pickaxe）");
            return HandlerResult::BadArgument;
        }
        if (method != "gold" && method != "ore") {
            ctx.msg("未知修复方式：" + method + "（可用 ore/gold）");
            return HandlerResult::BadArgument;
        }
        if (ctx.tools.is_full(id)) {
            ctx.msg(ctx.tools.name(id) + " 完好无损，无需修复。");
            return HandlerResult::Ok;
        }
        const int gold_cost = ctx.tools.gold_repair_cost(id);
        const std::string ore = ctx.tools.repair_ore(id);
        const int ore_needed = ctx.tools.repair_ore_count(id);

        if (method == "ore") {
            const int ore_cost = ctx.tools.ore_repair_cost(id);
            const int ore_held = ctx.player.GetBag().CountObject(ore);
            if (ore_held < ore_needed) {
                ctx.msg("矿石不足：" + ore + " 需要 x" + std::to_string(ore_needed)
                    + "，当前持有 " + std::to_string(ore_held) + "。");
                return HandlerResult::Failed;
            }
            if (ctx.gold < ore_cost) {
                ctx.msg("金币不足，矿石修复还需 " + std::to_string(ore_cost) + " 金币（当前 "
                    + std::to_string(ctx.gold) + "）。");
                return HandlerResult::Failed;
            }
            ctx.player.GetBag().RemoveObject(ore, ore_needed);
            ctx.gold -= ore_cost;
            ctx.player.SetState(StateCode::Repairing);
            ctx.tools.repair_full(id);
            ctx.player.SetState(StateCode::Waiting);
            ctx.msg("铁匠用 " + ore + " x" + std::to_string(ore_needed) + " + 金币 "
                + std::to_string(ore_cost) + " 修好了" + ctx.tools.name(id) + "。");
            return HandlerResult::Ok;
        }
        // 金币修复
        if (ctx.gold < gold_cost) {
            ctx.msg("金币不足：修复" + ctx.tools.name(id) + " 需要 " + std::to_string(gold_cost)
                + " 金币（当前 " + std::to_string(ctx.gold) + "）。");
            return HandlerResult::Failed;
        }
        ctx.gold -= gold_cost;
        ctx.player.SetState(StateCode::Repairing);
        ctx.tools.repair_full(id);
        ctx.player.SetState(StateCode::Waiting);
        ctx.msg("铁匠花费 " + std::to_string(gold_cost) + " 金币修好了" + ctx.tools.name(id) + "。");
        return HandlerResult::Ok;
    });
}