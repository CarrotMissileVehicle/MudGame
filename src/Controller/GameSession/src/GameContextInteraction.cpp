/**
 * @file GameContextInteraction.cpp
 * @brief GameContext 交互辅助实现：输入提示 / 反馈 / 通用输出 / 参数解析。
 *
 * DTO 装配（make_*_view）见 GameContext.cpp。
 */
#include "GameContext.h"

// 输入格式提示：根据玩家当前位置给出可用指令的行动名称，供每次输入前展示。
// 交互模式下用户只需输入行动名称（如 farm.sow），系统会逐个提示参数。
std::string GameContext::input_hint() const
{
    const char* place = nullptr;
    std::string cmds;
    switch (player.GetPosition())
    {
        case AtHome:
            place = "小屋（你的起点）";
            cmds = "  player.status —— 查看角色状态\n"
                   "  farm.status —— 查看农田概况\n"
                   "  fish.status —— 查看鱼池概况\n"
                   "  move.<up/down/left/right> —— 向对应方向移动";
            break;
        case AtFarmland:
            place = "农田";
            cmds = "  farm.status —— 查看农田\n"
                   "  farm.sow —— 播种\n"
                   "  farm.water —— 浇水\n"
                   "  farm.fertilize —— 施肥\n"
                   "  farm.harvest —— 收割\n"
                   "  move.<方向> —— 离开农田";
            break;
        case AtCoast:
            place = "海岸";
            cmds = "  fish.status —— 查看鱼池\n"
                   "  fish.tick —— 抛竿垂钓（每轮 3-6 秒，按 q 中止）\n"
                   "  move.<方向> —— 离开海岸";
            break;
        case AtMine:
            place = "矿洞";
            cmds = "  mine.status —— 查看采矿状态\n"
                   "  mine.start —— 开始采矿（每轮 3-6 秒，按 q 中止）\n"
                   "  mine.stop —— 手动结束采矿\n"
                   "  move.<方向> —— 离开矿洞";
            break;
        case AtTown:
            place = "小镇（集市所在地）";
            cmds = "  market.status —— 查看集市行情\n"
                   "  market.buy —— 购物\n"
                   "  market.sell —— 出售背包物品\n"
                   "  blacksmith.status —— 查看铁匠铺修复信息\n"
                   "  blacksmith.repair —— 修复工具\n"
                   "  move.<方向> —— 离开小镇";
            break;
    }
    return "【" + std::string(place) + "】当前可用指令：\n" + cmds
         + "\n通用指令：time.now 时间 | weather.now 天气 | tools.status 工具耐久"
           " | save 存档 | load 读档 | help 帮助 | quit 退出";
}

// 每次输入前先刷新输入法提示（TUI 在输入框上方展示）。
// 该提示仅在空闲（无输入、无补全、无问题）时显示。
void GameContext::refresh_prompt() const
{
    tui.input_hint = input_hint();
}

// 单行反馈文本：委托 MessagePanel 展示（进入 TUI 日志区）。
void GameContext::msg(const std::string& text) const
{
    view.render_message(mud::view::MessageLine{text});
}

// 通用输出：当前时刻 + 天气
void GameContext::render_now() const
{
    view.render_now(make_time_view(), make_weather_view());
}

// 从命令 options 读取整数型命名参数，缺省返回 fallback。
long long GameContext::opt_int(const mud::cmd::Command& cmd,
                               const std::string& key, long long fallback) const
{
    const auto it = cmd.options.find(key);
    if (it == cmd.options.end())
        return fallback;
    try { return std::stoll(it->second); }
    catch (...) { return fallback; }
}