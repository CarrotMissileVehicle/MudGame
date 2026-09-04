/**
 * @file input_parser_syntax_test.cpp
 * @brief 语法级测试（方案 CP-S）：分词、空白、大小写、别名、异常字符、超长、注入。
 *
 * 被测对象：InputParser（src/View/Cmdparser），CLI11 驱动。
 * 未实现特性（别名等）以 GTEST_SKIP 标注，避免误判为失败。
 */
#include <gtest/gtest.h>

#include <string>

#include "input_parser.h"

using mud::cmd::Command;

namespace {

// 抽取 verb 的便捷工具，供多条用例复用。
std::string parse_verb(const std::string& line)
{
    InputParser p;
    return p.parse(line).verb;
}

} // namespace

// CP-S-001 标准指令完整解析
TEST(InputParserSyntax, FullCommandParses)
{
    InputParser p;
    const Command c = p.parse("mine.start --layer 2");
    EXPECT_EQ(c.verb, "mine.start");
    EXPECT_EQ(c.options.at("layer"), "2");
    EXPECT_TRUE(c.args.empty());
    EXPECT_EQ(c.raw, "mine.start --layer 2");
}

// CP-S-002 空行输入
TEST(InputParserSyntax, EmptyLineYieldsEmptyVerb)
{
    EXPECT_EQ(parse_verb(""), "");
}

// CP-S-003 首尾空白
TEST(InputParserSyntax, LeadingTrailingWhitespace)
{
    InputParser p;
    const Command c = p.parse("  mine.start --layer 1  ");
    EXPECT_EQ(c.verb, "mine.start");
    EXPECT_EQ(c.options.at("layer"), "1");
}

// CP-S-004 连续多空白
TEST(InputParserSyntax, ConsecutiveWhitespace)
{
    InputParser p;
    const Command c = p.parse("mine.start     --layer    2");
    EXPECT_EQ(c.verb, "mine.start");
    EXPECT_EQ(c.options.at("layer"), "2");
}

// CP-S-005 Tab 分隔（有效分隔符或安全拒绝，二者皆不允许崩溃/误路由）
TEST(InputParserSyntax, TabSeparated)
{
    const auto verb = parse_verb("mine.start\t--layer\t2");
    EXPECT_TRUE(verb == "mine.start" || verb == "error")
        << "unexpected verb: " << verb;
}

// CP-S-006 缩写别名（仅当系统支持别名时执行）——当前未实现
TEST(InputParserSyntax, AliasNotImplementedSkipped)
{
    GTEST_SKIP()
        << "缩写别名 m->mine / n->north 尚未实现（方案 CP-S-006，仅当系统支持别名时执行）";
}

// CP-S-007 未知指令：go/look 现行未声明子命令，应安全落到 error 或空 verb
TEST(InputParserSyntax, UnknownGoNorth)
{
    const auto verb = parse_verb("go north");
    EXPECT_TRUE(verb == "error" || verb.empty()) << "unexpected verb: " << verb;
}

TEST(InputParserSyntax, UnknownLookRock)
{
    const auto verb = parse_verb("look rock");
    EXPECT_TRUE(verb == "error" || verb.empty()) << "unexpected verb: " << verb;
}

// CP-S-008 --help 帮助请求（根）
TEST(InputParserSyntax, RootHelpRequest)
{
    EXPECT_EQ(parse_verb("--help"), "help");
}

// CP-S-008b --help（子命令）
TEST(InputParserSyntax, SubcommandHelpRequest)
{
    EXPECT_EQ(parse_verb("mine.start --help"), "help");
}

// CP-S-009 超长输入不崩溃
TEST(InputParserSyntax, OversizedInputDoesNotThrow)
{
    std::string line = "mine.start --layer 1" + std::string(20000, 'x');
    EXPECT_NO_THROW({ (void)parse_verb(line); });
}

// CP-S-010 非法选项被拒绝
TEST(InputParserSyntax, UnknownOptionRejected)
{
    EXPECT_EQ(parse_verb("mine.start --bogus 3"), "error");
}

// CP-S-011 缺少必填参数
TEST(InputParserSyntax, MissingRequiredLayer)
{
    EXPECT_EQ(parse_verb("mine.start"), "error");
}

TEST(InputParserSyntax, MissingRequiredFactor)
{
    EXPECT_EQ(parse_verb("time.scale"), "error");
}

// CP-S-012 SQL 注入特征字符仅作为数据，不产生任何执行（此处校验不崩溃、verb 合法）
TEST(InputParserSyntax, SqlInjectionStaysData)
{
    const auto c = InputParser{}.parse(
        "mine.start --layer 1'; DROP TABLE ore; --");
    EXPECT_TRUE(c.verb == "mine.start" || c.verb == "error")
        << "unexpected verb: " << c.verb;
}

// CP-S-013 脚本注入特征：无 shell 执行、不崩溃
TEST(InputParserSyntax, ScriptInjectionNoShell)
{
    EXPECT_NO_THROW({ (void)parse_verb("mine.start --layer \"$(rm -rf /)\""); });
    EXPECT_NO_THROW({ (void)parse_verb("quit && rm -f *"); });
    EXPECT_NO_THROW({ (void)parse_verb("mine.start --layer \"1 | cat /etc/passwd\""); });
}

// CP-S-014 Emoji / 多字节输入
TEST(InputParserSyntax, EmojiInputNoCrash)
{
    EXPECT_NO_THROW({ (void)parse_verb("⛏ mine.start --layer 1 ⛏"); });
    EXPECT_NO_THROW({ (void)parse_verb("MINE ⛏ COPPER"); });
}

// CP-S-015 特殊分隔符 / 混合符号
TEST(InputParserSyntax, SpecialDelimiters)
{
    EXPECT_NO_THROW({ (void)parse_verb("mine.start ,;:!@#$%^&*() = --layer 1"); });
    EXPECT_NO_THROW({ (void)parse_verb("mine.start --layer 1_-.()"); });
}

// CP-S-016 大小写：Connector 路由层不敏感（见逻辑 CP-L-004），此处校验 parser 不崩溃
TEST(InputParserSyntax, CaseMixingNoCrash)
{
    EXPECT_NO_THROW({ (void)parse_verb("MINE.START --layer 1"); });
    EXPECT_NO_THROW({ (void)parse_verb("Time.Now"); });
    EXPECT_NO_THROW({ (void)parse_verb("QUIT"); });
}