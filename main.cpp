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
    bool to_size(const std::string& s, std::size_t& out)
    {
        if (s.find_first_not_of("0123456789") != std::string::npos) return false;
        char* end = nullptr;
        errno = 0;
        out = std::strtoull(s.c_str(), &end, 10);
        return end && *end == '\0' && errno == 0;
    }

    bool to_double(const std::string& s, double& out)
    {
        char* end = nullptr;
        errno = 0;
        out = std::strtod(s.c_str(), &end);
        return end && *end == '\0' && errno == 0;
    }

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

int main()
{
    mud::TimeService time_service;
    ::MiningState session;
    mud::mining::MiningContext mctx;

    InputParser parser;
    Connector connector;

    // 命令绑定
    connector.bind("mine.start", [&mctx](const mud::cmd::Command& c, const HandlerContext& ctx)
    {
        std::size_t layer = 0;
        auto it = c.options.find("layer");
        if (it == c.options.end() || !to_size(it->second, layer)) return HandlerResult::BadArgument;
        mctx = mud::mining::MiningContext{}; // 重置上下文
        ctx.session.start(layer, ctx.time.now());
        std::cout << "开始采矿：层 " << layer << "\n";
        return HandlerResult::Ok;
    });

    connector.bind("mine.stop", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        ctx.session.stop();
        std::cout << "已停止采矿\n";
        return HandlerResult::Ok;
    });

    connector.bind("mine.status", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        if (!ctx.session.is_mining())
        {
            std::cout << "当前未在采矿\n";
            return HandlerResult::Ok;
        }
        const auto& layer = ctx.session.layer_id();
        std::cout << "采矿中：层 " << (layer ? std::to_string(*layer) : "?")
            << "，开始时间=" << ctx.session.start_time().time_since_epoch().count() << "\n";
        return HandlerResult::Ok;
    });

    connector.bind("time.now", [](const mud::cmd::Command&, const HandlerContext& ctx)
    {
        std::cout << "当前时间=" << ctx.time.now().time_since_epoch().count() << "\n";
        return HandlerResult::Ok;
    });

    connector.bind("time.scale", [](const mud::cmd::Command& c, const HandlerContext& ctx)
    {
        double f = 0.0;
        auto it = c.options.find("factor");
        if (it == c.options.end() || !to_double(it->second, f)) return HandlerResult::BadArgument;
        ctx.time.set_time_scale(f);
        std::cout << "时间倍率=" << ctx.time.time_scale() << "\n";
        return HandlerResult::Ok;
    });

    std::string line;
    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        auto cmd = parser.parse(line);
        if (cmd.verb == "quit") break;
        if (cmd.verb == "help") { std::cout << parser.help_text(); continue; }
        if (cmd.verb.empty() || cmd.verb == "error") continue;

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
