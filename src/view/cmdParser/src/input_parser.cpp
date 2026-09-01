#include "input_parser.h"

#include <CLI/CLI.hpp>

#include <sstream>
#include <string_view>

/* =====================================================
 * InputParser：命令行文本解析器实现（桩代码）
 *
 * 职责：把一行原始输入解析为结构化的 mud::cmd::Command。
 *   Command.verb    —— 首词（命令动词）
 *   Command.args    —— 位置参数（动词之后的非命名词）
 *   Command.options —— 命名参数 / 选项（--key 或 -k value 形式）
 *   Command.raw     —— 原始输入整行
 * ===================================================== */

// 解析一行输入为 Command
//
// 解析策略（待实现）：
//   1. 以空白字符切分输入行；
//   2. 第一个 token 填入 verb；
//   3. 形如 --name / -name 且后跟值的 token 解析进 options；
//   4. 其余 token 依次追加到 args。
//   5. 若存在不成对的选项（缺值）、括号不匹配等，视为解析失败并抛出 ParseError。
mud::cmd::Command InputParser::parse(const std::string& line) const
{
    mud::cmd::Command cmd;
    cmd.raw = line;  // 原始行原样保存

    // TODO: 实现分词与选项/位置参数解析逻辑
    //   std::istringstream iss(line);
    //   std::string token;
    //   bool first = true;
    //   while (iss >> token) {
    //       if (first) { cmd.verb = token; first = false; }
    //       else if (token.size() >= 2 && token[0] == '-') { ...options... }
    //       else { cmd.args.push_back(token); }
    //   }

    return cmd;
}