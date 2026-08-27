#pragma once

/*
raw_line → InputParser::parse → Command
        → Connector::dispatch → (按 verb 查表) → 领域 handler
        → 调 MiningController 等 → 反馈文本 → 回 main 打印
 */

#include "time_service.h"
#include "mining_controller.h"

namespace mud::cmd
{
    struct Command;
}

enum class HandlerResult { Ok, UnknownCommand, BadArgument, Failed };

struct HandlerContext {  // 透传给领域 handler 的运行时依赖
    const TimeService& time;
    MiningState& session;
};

using Handler = std::function<
    HandlerResult(
        const cmd::Command&,
        const HandlerContext&
        )
>;

class Connector {
private:
    std::unordered_map<std::string, Handler> handlers_;

public:
    void bind(std::string verb, Handler handler);

    HandlerResult dispatch(
        const std::string& raw_line,
        const HandlerContext& ctx) const;

    bool has(std::string_view verb) const;
    // throws UnknownCommand / ParseError
};