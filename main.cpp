/**
 * @file main.cpp
 * @brief MUD 游戏命令行入口。
 *
 * 负责装配游戏核心组件（时间服务、采矿会话）、注册命令处理器，
 * 并运行「游戏帧循环（1 现实秒 1 帧）+ 非阻塞读输入 -> 解析命令 -> 派发执行」的主循环。
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
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
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

    /**
     * @brief 将游戏时间格式化为 "YYYY-MM-DD HH:MM"（无秒）。
     * @param t 游戏时间。
     * @return 可读的日历字符串。
     */
    std::string format_time(const mud::time::GameDateTime& t)
    {
        std::ostringstream os;
        os << t.year << '-'
           << std::setw(2) << std::setfill('0') << t.month << '-'
           << std::setw(2) << std::setfill('0') << t.day << ' '
           << std::setw(2) << std::setfill('0') << t.hour << ':'
           << std::setw(2) << std::setfill('0') << t.minute;
        return os.str();
    }
} // namespace

/**
 * @brief 程序主入口。
 *
 * 装配核心对象、绑定各类游戏命令，然后进入 1 现实秒 1 帧的游戏帧循环。
 * 时间服务由循环每帧推进（现实 1s = 游戏 60s，受倍率影响）；输入由读取
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
        const auto& layer = ctx.session.layer_id();
        std::cout << "采矿中：层 " << (layer ? std::to_string(*layer) : "?")
            << "，开始时间=" << format_time(ctx.session.start_time()) << "\n";
        return HandlerResult::Ok;
    });

    // 查询游戏当前时间（日历显示，无秒）。
    connector.bind("time.now", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        std::cout << "当前时间=" << format_time(ctx.time.now()) << "\n";
        return HandlerResult::Ok;
    });

    // 设置时间流速倍率（游戏秒/现实秒）。
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

    // 游戏帧循环：固定 1000ms（1 现实秒）一拍。每拍推进游戏时间，
    // 触发到期回调，并排空输入队列里的命令。
    constexpr auto frame_dur = std::chrono::milliseconds(1000);
    auto last = std::chrono::steady_clock::now();
    bool running = true;

    while (running)
    {
        const auto frame_now = std::chrono::steady_clock::now();
        last = frame_now;

        time_service.update();

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

            HandlerContext ctx{time_service, session};
            auto r = connector.dispatch(cmd, ctx);
            std::cout << "[结果: " << result_text(r) << "]\n";
        }

        std::this_thread::sleep_until(last + frame_dur);
    }

    reader.join();
    std::cout << "再见\n";
    return 0;
}