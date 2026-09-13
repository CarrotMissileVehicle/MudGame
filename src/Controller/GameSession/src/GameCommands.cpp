/**
 * @file GameCommands.cpp
 * @brief GameCommands 实现：register_all 聚合入口 + 参数 Schema 注册。
 */
#include "GameCommands.h"

#include "GameContext.h"
#include "WorldEngine.h"
#include "connector.h"

void GameCommands::register_all(Connector& connector, GameContext& ctx, WorldEngine& world)
{
    register_mining(connector, ctx, world);
    register_farm(connector, ctx);
    register_market(connector, ctx);
    register_blacksmith(connector, ctx);
    register_misc(connector, ctx, world);
    register_schemas(connector, ctx);
}

void GameCommands::register_schemas(Connector& connector, GameContext& ctx)
{
    const std::size_t farmSize = ctx.farming.farmSize();
    const std::string plotRange = "地块索引(0-" + std::to_string(farmSize - 1) + ")";

    connector.register_schema("mine.start", {
        "开始采矿",
        {{"layer", "目标层(0-4)", true, ""}}
    });

    connector.register_schema("time.scale", {
        "设置时间倍率",
        {{"factor", "时间倍率(如 0.5/1/2/60)", true, ""}}
    });

    connector.register_schema("farm.sow", {
        "播种",
        {{"plot", plotRange, true, ""},
         {"crop", "作物名(cabbage/carrot/tomato/pumpkin/lingzhi)", true, ""}}
    });

    connector.register_schema("farm.water", {
        "浇水",
        {{"plot", plotRange, true, ""}}
    });

    connector.register_schema("farm.fertilize", {
        "施肥",
        {{"plot", plotRange, true, ""},
         {"type", "肥料类型(normal/advanced)", true, ""}}
    });

    connector.register_schema("farm.harvest", {
        "收割",
        {{"plot", plotRange, true, ""}}
    });

    connector.register_schema("market.buy", {
        "从商店购买",
        {{"shop", "商店ID(seed/grocery)", true, ""},
         {"item", "物品名", true, ""},
         {"count", "购买数量", false, "1"}}
    });

    connector.register_schema("market.sell", {
        "向集市出售背包物品",
        {{"item", "物品名或序号", true, ""},
         {"count", "出售数量", false, "1"}}
    });

    connector.register_schema("blacksmith.repair", {
        "在铁匠铺修复工具",
        {{"tool", "工具名(hoe/rod/pickaxe)", true, ""},
         {"method", "修复方式(ore/gold)", true, ""}}
    });
}