/**
 * @file main.cpp
 * @brief MUD 游戏命令行入口。
 *
 * 负责装配游戏核心组件（时间服务、采矿会话）、注册命令处理器，
 * 并运行「游戏帧循环 + 非阻塞读输入 -> 解析命令 -> 派发执行」的主循环。
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
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

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
 * 装配核心对象、绑定各类游戏命令，然后进入带提示符的游戏帧循环。
 * 时间服务由循环每帧按真实经过时长推进（受时间倍率影响）；输入由读取
 * 线程推入队列，主循环每帧非阻塞排空并派发命令；quit 或 EOF 退出。
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

    // 输入读取线程：把 stdin 逐行推入线程安全队列，供帧循环非阻塞排空。
    std::queue<std::string> input_q;
    std::mutex q_mtx;
    std::condition_variable q_cv;
    bool input_open = true;

    std::thread reader([&]
    {
        std::string line;
        while (std::getline(std::cin, line))
        {
            std::lock_guard<std::mutex> lk(q_mtx);
            input_q.push(std::move(line));
        }
        std::lock_guard<std::mutex> lk(q_mtx);
        input_open = false;
        q_cv.notify_one();
    });

    // 游戏帧循环：固定 100ms 一拍。每拍按真实经过时长推进游戏时间，
    // 触发到期回调，并排空输入队列里的命令。
    constexpr auto frame_dur = std::chrono::milliseconds(100);
    auto last = std::chrono::steady_clock::now();
    bool running = true;

    while (running)
    {
        const auto frame_now = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<mud::time::gameDuration>(frame_now - last);
        last = frame_now;

        time_service.update(delta);

        // 排空已就绪的输入
        for (;;)
        {
            std::string line;
            {
                std::unique_lock<std::mutex> lk(q_mtx);
                if (input_q.empty())
                {
                    if (!input_open) running = false; // EOF：结束后退出
                    break;
                }
                line = std::move(input_q.front());
                input_q.pop();
            }
            if (line.empty()) continue;

            auto cmd = parser.parse(line);
            if (cmd.verb == "quit") { running = false; break; }
            if (cmd.verb == "help") { std::cout << parser.help_text(); continue; }
            if (cmd.verb.empty() || cmd.verb == "error") continue;

            std::cerr << "[TRACE before dispatch verb=" << cmd.verb << "]\n";
            HandlerContext ctx{time_service, session};
            std::cerr << "[TRACE ctx built]\n";
            auto r = connector.dispatch(cmd, ctx);
            std::cerr << "[TRACE after dispatch]\n";
            std::cout << "[结果: " << result_text(r) << "]\n";
        }

        std::this_thread::sleep_until(last + frame_dur);
    }

    reader.join();
    std::cout << "再见\n";
    return 0;
}