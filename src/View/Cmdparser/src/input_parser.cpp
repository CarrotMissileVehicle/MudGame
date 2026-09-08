/**
 * @file input_parser.cpp
 * @brief 命令行解析器实现。
 *
 * 基于 CLI11 声明式 schema 定义各子命令及选项，并将输入行转换为
 * 结构化的 mud::cmd::Command 对象。
 */
#include "input_parser.h"

#include <string>
#include <vector>

/**
 * @brief 构造函数：声明命令解析 schema。
 *
 * 使用点分动词作为各子命令名（如 mine.start），并为其声明命名参数。
 * 带必选参数的子命令用 required() 标记。
 */
InputParser::InputParser()
{
    auto* start = app_.add_subcommand("mine.start", "开始采矿");
    start->add_option("--layer", layer_, "目标层(0-4)")->required();

    app_.add_subcommand("mine.stop", "停止采矿");
    app_.add_subcommand("mine.status", "查询采矿状态");

    app_.add_subcommand("time.now", "查询当前时间");
    auto* scale = app_.add_subcommand("time.scale", "设置时间倍率");
    scale->add_option("--factor", factor_, "时间倍率")->required();

    app_.add_subcommand("player.status", "查看玩家状态");

    for (const char* dir : {"up", "down", "left", "right"}) {
        app_.add_subcommand("move." + std::string(dir), std::string("向") + dir + "移动");
    }

    app_.add_subcommand("farm.status", "查看农田状态");
    auto* sow = app_.add_subcommand("farm.sow", "播种");
    sow->add_option("--plot", plot_, "地块索引(0起)")->required();
    sow->add_option("--crop", crop_, "作物名(cabbage/carrot/tomato/pumpkin/lingzhi)")->required();
    auto* water = app_.add_subcommand("farm.water", "浇水");
    water->add_option("--plot", plot_, "地块索引(0起)")->required();
    auto* fert = app_.add_subcommand("farm.fertilize", "施肥");
    fert->add_option("--plot", plot_, "地块索引(0起)")->required();
    fert->add_option("--type", fert_type_, "肥料类型(normal/advanced)")->required();
    auto* harvest = app_.add_subcommand("farm.harvest", "收割");
    harvest->add_option("--plot", plot_, "地块索引(0起)")->required();

    app_.add_subcommand("fish.status", "查看钓鱼情况");
    app_.add_subcommand("fish.tick", "尝试钓鱼");

    app_.add_subcommand("weather.now", "查看今日天气");

    app_.add_subcommand("market.status", "查看集市行情");
    auto* buy = app_.add_subcommand("market.buy", "从商店购买");
    buy->add_option("--shop", shop_, "商店ID(seed/grocery)")->required();
    buy->add_option("--item", item_, "物品名")->required();
    buy->add_option("--count", count_, "购买数量")->default_str("1");
    auto* sell = app_.add_subcommand("market.sell", "向集市出售背包物品");
    sell->add_option("--item", item_, "物品名")->required();
    sell->add_option("--count", count_, "出售数量")->default_str("1");

    app_.add_subcommand("tools.status", "查看工具耐久");

    app_.add_subcommand("help", "显示帮助");
    app_.add_subcommand("quit", "退出游戏");
    app_.add_subcommand("save", "保存游戏");
}

/**
 * @brief 解析单行输入为命令对象。
 * @param line 用户输入的命令行。
 * @return 解析结果；空行返回空 verb，解析错误返回 verb="error"，--help 返回 verb="help"。
 */
mud::cmd::Command InputParser::parse(const std::string& line)
{
    mud::cmd::Command cmd;
    cmd.raw = line;
    if (line.empty()) return cmd; // 空输入 → verb 为空

    app_.clear(); // 关键：每轮归零解析状态，避免上一轮残留选项
    try
    {
        // 字符串入口（本代 CLI11 的 parse(vector) 有缺陷：不消费选项值），第二参 false=不含程序名
        app_.parse(line, false);
    }
    catch (const CLI::ParseError& e)
    {
        if (e.get_name() == "CallForHelp")
        {
            cmd.verb = "help"; // --help 触发：不在此打印，交由 main 统一打印 help_text()
            return cmd;
        }
        (void)app_.exit(e); // 打印参数错误到 stdout，绝不退出进程
        cmd.verb = "error";
        return cmd;
    }

    // 扁平设计：根上已解析的子命令（至多一个），提取动词、命名选项与剩余位置参数
    const auto subs = app_.get_subcommands(); // parsed_subcommands_
    if (!subs.empty())
    {
        CLI::App* leaf = subs.front();
        cmd.verb = leaf->get_name();
        for (const auto* opt : leaf->get_options())
        {
            if (opt->count() == 0) continue;
            // 去掉选项名前导 '-'，统一存入 options 映射
            std::string name = opt->get_name();
            while (name.size() > 1 && name[0] == '-') name = name.substr(1);
            cmd.options[name] = opt->as<std::string>();
        }
        cmd.args = leaf->remaining();
    }
    return cmd;
}

/** @brief 返回 CLI11 生成的完整帮助文本。 */
std::string InputParser::help_text() const { return app_.help(); }