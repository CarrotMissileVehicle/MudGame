#include "input_parser.h"

#include <string>
#include <vector>

InputParser::InputParser()
{
    auto* start = app_.add_subcommand("mine.start", "开始采矿");
    start->add_option("--layer", layer_, "目标层(0-4)")->required();

    app_.add_subcommand("mine.stop", "停止采矿");
    app_.add_subcommand("mine.status", "查询采矿状态");

    app_.add_subcommand("time.now", "查询当前时间");
    auto* scale = app_.add_subcommand("time.scale", "设置时间倍率");
    scale->add_option("--factor", factor_, "时间倍率")->required();

    app_.add_subcommand("help", "显示帮助");
    app_.add_subcommand("quit", "退出游戏");
    app_.add_subcommand("save", "保存游戏");
}

mud::cmd::Command InputParser::parse(const std::string& line)
{
    mud::cmd::Command cmd;
    cmd.raw = line;
    if (line.empty()) return cmd; // 空输入 → verb 为空

    app_.clear(); // 关键：每轮归零解析状态
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

    // 扁平设计：根上已解析的子命令（至多一个）
    const auto subs = app_.get_subcommands(); // parsed_subcommands_
    if (!subs.empty())
    {
        CLI::App* leaf = subs.front();
        cmd.verb = leaf->get_name();
        for (const auto* opt : leaf->get_options())
        {
            if (opt->count() == 0) continue;
            std::string name = opt->get_name();
            while (name.size() > 1 && name[0] == '-') name = name.substr(1);
            cmd.options[name] = opt->as<std::string>();
        }
        cmd.args = leaf->remaining();
    }
    return cmd;
}

std::string InputParser::help_text() const { return app_.help(); }