#include "connector.h"
#include "input_parser.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

void Connector::bind(std::string verb, Handler handler)
{
    std::transform(verb.begin(), verb.end(), verb.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    handlers_[std::move(verb)] = std::move(handler);
}

HandlerResult Connector::dispatch(const mud::cmd::Command& command,
                                  const HandlerContext& ctx) const
{
    std::string verb = command.verb;
    std::transform(verb.begin(), verb.end(), verb.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    auto it = handlers_.find(verb);
    if (it == handlers_.end()) return HandlerResult::UnknownCommand;
    return it->second(command, ctx);
}

bool Connector::has(std::string_view verb) const
{
    return handlers_.find(std::string(verb)) != handlers_.end();
}
