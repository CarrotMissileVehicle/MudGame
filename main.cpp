/**
 * @file main.cpp
 * @brief MUD 游戏命令行入口。
 *
 * 负责装配游戏核心组件（时间服务、采矿会话）、注册命令处理器，
 * 并运行「读取输入 -> 解析命令 -> 派发执行」的主循环。
 *
 * 依赖：cmdParser(InputParser/Connector)、mining_controller(MiningState)、
 *       timeService(TimeService)。
 */

#include "connector.h"
#include "input_parser.h"
#include "mining_state.h"
#include "mining_types.h"
#include "time_service.h"

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    /**
     * @brief 将字符串安全转换为无符号整数。
     * @param s  待解析的字符串，仅允许十进制数字。
     * @param out 解析成功的输出值。
     * @return 解析成功返回 true，否则返回 false。
     */
    bool to_size(const std::string& s, std::size_t& out)
    {
        // 字符串含非数字字符即失败，随后用 strtoull 全量解析并校验吞尾。
        if (s.find_first_not_of("0123456789") != std::string::npos) return false;
        char* end = nullptr;
        errno = 0;
        out = std::strtoull(s.c_str(), &end, 10);
        return end && *end == '\0' && errno == 0;
    }

    /**
     * @brief 将字符串安全转换为浮点数。
     * @param s  待解析的字符串。
     * @param out 解析成功的输出值。
     * @return 解析成功返回 true，否则返回 false。
     */
    bool to_double(const std::string& s, double& out)
    {
        char* end = nullptr;
        errno = 0;
        out = std::strtod(s.c_str(), &end);
        return end && *end == '\0' && errno == 0;
    }

    /**
     * @brief 将命令处理结果枚举映射为中文提示文本。
     * @param r 命令处理结果。
     * @return 对应的中文描述字符串。
     */
    const char* result_text(HandlerResult r)
    {
        switch (r)
        {
        case HandlerResult::Ok: return "成功";
        case HandlerResult::BadArgument: return "参数无效";
        case HandlerResult::UnknownCommand: return "未知命令";
        case HandlerResult::Failed: return "执行失败";
        }
        return "";
    }
} // namespace

/**
 * @brief 程序主入口。
 *
 * 装配核心对象、绑定各类游戏命令，然后进入带提示符的命令循环。
 * 通过 stdin 逐行读取命令，解析并派发到对应处理器；遇到 quit 或 EOF 退出。
 *
 * @return 进程返回值，正常退出为 0。
 */
int main()
{
    mud::TimeService time_service;
    ::MiningState session;
    mud::mining::MiningContext mctx;

    InputParser parser;
    Connector connector;

    // 命令绑定：将命令动词与其处理回调及所需参数关联。
    // 参数直接提取自 Command.options；合法则执行，否则返回 BadArgument。
    // 注意：mine.start 需重置 MiningContext，保证每次开始时是全新会话。
    connector.bind("mine.start", [&mctx](const mud::cmd::Command& c, const HandlerContext& ctx)
    {
        std::size_t layer = 0;
        auto it = c.options.find("layer");
        if (it == c.options.end() || !to_size(it->second, layer)) return HandlerResult::BadArgument;
        mctx = mud::mining::MiningContext{}; // 重置上下文，保证每次开始均为全新采矿会话。
        ctx.session.start(layer, ctx.time.now());
        std::cout << "开始采矿：层 " << layer << "\n";
        return HandlerResult::Ok;
    });

    // 停止当前采矿会话，无参数。
    connector.bind("mine.stop", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        ctx.session.stop();
        std::cout << "已停止采矿\n";
        return HandlerResult::Ok;
    });

    // 查询采矿状态：未采矿时给出提示；采矿中则输出当前层与开始时间。
    connector.bind("mine.status", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        if (!ctx.session.is_mining())
        {
            std::cout << "当前未在采矿\n";
            return HandlerResult::Ok;
        }
        // layer_id 存于 std::optional 中，空则显示 "?"。
        const auto& layer = ctx.session.layer_id();
        std::cout << "采矿中：层 " << (layer ? std::to_string(*layer) : "?")
            << "，开始时间=" << ctx.session.start_time().time_since_epoch().count() << "\n";
        return HandlerResult::Ok;
    });

    // 查询游戏当前时间（以时钟周期计数输出）。
    connector.bind("time.now", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        std::cout << "当前时间=" << ctx.time.now().time_since_epoch().count() << "\n";
        return HandlerResult::Ok;
    });

    // 设置时间流速倍率：factor 参数非法时返回 BadArgument。
    connector.bind("time.scale", [](const mud::cmd::Command& c, const HandlerContext& ctx)
    {
        double f = 0.0;
        auto it = c.options.find("factor");
        if (it == c.options.end() || !to_double(it->second, f)) return HandlerResult::BadArgument;
        ctx.time.set_time_scale(f);
        std::cout << "时间倍率=" << ctx.time.time_scale() << "\n";
        return HandlerResult::Ok;
    });

    // 主命令循环：解析 stdin 输入并派发至已绑定处理器。
    // 逐行读取，忽略空行；quit 退出、help 打印帮助、error/空动词则跳过。
    std::string line;
    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break; // EOF 时结束循环
        if (line.empty()) continue;

        auto cmd = parser.parse(line);
        if (cmd.verb == "quit") break;
        if (cmd.verb == "help") { std::cout << parser.help_text(); continue; }
        if (cmd.verb.empty() || cmd.verb == "error") continue;

        // 为每次派发建立独立的 HandlerContext，绑定时间服务与会话状态。
        std::cerr << "[TRACE before dispatch verb=" << cmd.verb << "]\n";
        HandlerContext ctx{time_service, session};
        std::cerr << "[TRACE ctx built]\n";
        auto r = connector.dispatch(cmd, ctx);
        std::cerr << "[TRACE after dispatch]\n";
        std::cout << "[结果: " << result_text(r) << "]\n";
    }

    std::cout << "再见\n";
    return 0;
}
