/**
 * @file CommandCatalog.cpp
 * @brief CommandCatalog 实现：命令集合、帮助文本与参数候选（源自原 main.cpp）。
 */
#include "CommandCatalog.h"

#include "GameContext.h"

std::vector<std::string> CommandCatalog::known_verbs()
{
    return {
        "time.now", "time.scale", "player.status",
        "move.up", "move.down", "move.left", "move.right",
        "farm.status", "farm.sow", "farm.water", "farm.fertilize", "farm.harvest",
        "fish.status", "fish.tick", "weather.now",
        "mine.status", "mine.start", "mine.stop",
        "market.status", "market.buy", "market.sell",
        "tools.status", "blacksmith.status", "blacksmith.repair",
        "save", "load", "help", "quit"
    };
}

std::string CommandCatalog::help_text()
{
    return
        "可用指令（输入行动名称后按提示填写参数）：\n"
        "  time.now —— 查看当前时间\n"
        "  time.scale —— 设置时间倍率\n"
        "  player.status —— 查看角色状态\n"
        "  move.up / move.down / move.left / move.right —— 移动\n"
        "  farm.status —— 查看农田\n"
        "  farm.sow —— 播种（需地块索引和作物名）\n"
        "  farm.water —— 浇水（需地块索引）\n"
        "  farm.fertilize —— 施肥（需地块索引和肥料类型）\n"
        "  farm.harvest —— 收割（需地块索引）\n"
        "  fish.status —— 查看鱼池\n"
        "  fish.tick —— 开始钓鱼（q 结束）\n"
        "  weather.now —— 查看天气\n"
        "  mine.status —— 查看采矿状态\n"
        "  mine.start —— 开始采矿（需层号，q 结束）\n"
        "  mine.stop —— 停止采矿\n"
        "  market.status —— 查看集市\n"
        "  market.buy —— 购物（需商店/物品/数量）\n"
        "  market.sell —— 出售（需物品/数量）\n"
        "  tools.status —— 查看工具耐久\n"
        "  blacksmith.status —— 查看铁匠铺\n"
        "  blacksmith.repair —— 修复工具（需工具名/方式）\n"
        "  save —— 存档  |  load —— 读档  |  quit —— 退出";
}

std::vector<std::string> CommandCatalog::param_choices(
    const mud::cmd::ParameterDef& p, const GameContext& ctx)
{
    if (p.name == "layer")
        return {"0", "1", "2", "3", "4"};
    if (p.name == "crop")
        return {"cabbage", "carrot", "tomato", "pumpkin", "lingzhi"};
    if (p.name == "type")
        return {"normal", "advanced"};
    if (p.name == "shop")
        return {"seed", "grocery"};
    if (p.name == "tool")
        return {"hoe", "rod", "pickaxe"};
    if (p.name == "method")
        return {"ore", "gold"};
    if (p.name == "plot") {
        std::vector<std::string> out;
        for (std::size_t i = 0; i < ctx.farming.farmSize(); ++i)
            out.push_back(std::to_string(i));
        return out;
    }
    return {};
}