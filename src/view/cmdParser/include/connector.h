#pragma once

/*
line → InputParser::parse → Command(点分 verb)
     → Connector::dispatch → (按 verb 查表) → 领域 handler
     → HandlerResult → main 打印反馈
 */

#include "time_service.h"
#include "mining_state.h"
#include "input_parser.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

enum class HandlerResult { Ok, UnknownCommand, BadArgument, Failed };

struct HandlerContext
{
    // 透传给领域 handler 的运行时依赖
    mud::TimeService& time; // 非 const：time scale 需调用 set_time_scale
    ::MiningState& session;
};

using Handler = std::function<
    HandlerResult(const mud::cmd::Command&, const HandlerContext&)>;

class Connector
{
private:
    std::unordered_map<std::string, Handler> handlers_;

public:
    void bind(std::string verb, Handler handler);

    HandlerResult dispatch(
        const mud::cmd::Command& command,
        const HandlerContext& ctx) const;

    bool has(std::string_view verb) const;
};
