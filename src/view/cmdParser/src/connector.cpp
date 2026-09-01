#include "connector.h"
#include "input_parser.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

/* =====================================================
 * Connector：命令分发器实现（桩代码）
 *
 * 职责：把原始命令行按动词(verb)路由到已绑定的领域 handler。
 * 流程见 connector.h 顶部注释：
 *   raw_line → InputParser::parse → Command
 *           → Connector::dispatch → (按 verb 查表) → 领域 handler
 *           → 调 MiningController 等 → 反馈文本 → 回 main 打印
 *
 * NOTE: 当前实现为占位桩，方法体仅返回安全默认值，待后续补全。
 * ===================================================== */

// 绑定命令动词到处理函数
//
// 将 verb 作为键存入 handlers_ 表，后续 dispatch 时据此查找。
// TODO: 实现 —— 形如 handlers_[std::move(verb)] = std::move(handler);
//       注意：handler 以 std::function 存储（可复制），verb 键统一小写便于匹配。
void Connector::bind(std::string verb, Handler handler)
{
    // TODO: 实现绑定逻辑
    (void)verb;
    (void)handler;
}

// 解析原始行并按动词分发命令
//
// 先交由 InputParser 解析为 Command，再从 handlers_ 中按 verb 查找对应 handler，
// 命中则调用 handler(command, ctx)，未命中返回 HandlerResult::UnknownCommand。
// 解析语法失败时按约定抛出 ParseError；查找不到也可抛出 UnknownCommand。
// 因本方法为 const，handlers_ 仅在查找层被读取，不修改其内容。
HandlerResult Connector::dispatch(
    const std::string& raw_line,
    const HandlerContext& ctx) const
{
    // TODO: 实现分发逻辑
    //   InputParser parser;
    //   const cmd::Command command = parser.parse(raw_line);
    //   auto it = handlers_.find(command.verb);
    //   if (it == handlers_.end()) return HandlerResult::UnknownCommand;
    //   return it->second(command, ctx);
    (void)raw_line;
    (void)ctx;
    return HandlerResult::UnknownCommand;
}

// 查询某动词是否已注册
//
// 仅做只读成员查询：handler 表存在该 verb 键即返回 true。
// TODO: 实现 —— 形如 return handlers_.find(verb) != handlers_.end();
bool Connector::has(std::string_view verb) const
{
    // TODO: 实现查询逻辑
    (void)verb;
    return false;
}