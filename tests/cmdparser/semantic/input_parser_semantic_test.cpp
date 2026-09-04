/**
 * @file input_parser_semantic_test.cpp
 * @brief 语义级测试（方案 CP-M）：参数识别、取值范围、类型与资源语义。
 *
 * 被测对象：InputParser 的参数提取与语义拒绝；
 * 层号/等级/照明/越界等由 MiningController 决策，相关边界在
 * 上下文级 state_gate_test.cpp 中覆盖。
 */
#include <gtest/gtest.h>

#include <string>

#include "input_parser.h"

using mud::cmd::Command;

namespace {

std::string parse_verb(const std::string& line)
{
    return InputParser{}.parse(line).verb;
}

} // namespace

// CP-M-001 合法层号（下/上边界）被解析
TEST(InputParserSemantic, LayerBoundsExtracted)
{
    for (const auto& layer : {"0", "4"})
    {
        const Command c = InputParser{}.parse(
            std::string("mine.start --layer ") + layer);
        EXPECT_EQ(c.verb, "mine.start");
        EXPECT_EQ(c.options.at("layer"), layer);
    }
}

// CP-M-002/003 层号越界（5 / -1）：parser 接受为选项值，范围拦截在控制器（见 context 测试）
TEST(InputParserSemantic, OutOfRangeLayerExtractedByParser)
{
    const Command c5 = InputParser{}.parse("mine.start --layer 5");
    EXPECT_EQ(c5.verb, "mine.start");
    EXPECT_EQ(c5.options.at("layer"), "5");
}

// CP-M-004 层号非数字：类型转换失败 → error
TEST(InputParserSemantic, NonNumericLayerRejected)
{
    EXPECT_EQ(parse_verb("mine.start --layer abc"), "error");
    EXPECT_EQ(parse_verb("mine.start --layer 1.5"), "error");
    // 前导空白数字：CLI11 数字转换会裁剪空白并接受；数值归一/越界拦截在控制器
    EXPECT_EQ(parse_verb("mine.start --layer \" 3\""), "mine.start");
}

// CP-M-007/008 --factor 缺省与类型
TEST(InputParserSemantic, FactorMissingRejected)
{
    EXPECT_EQ(parse_verb("time.scale"), "error");
}

TEST(InputParserSemantic, FactorTypeRejected)
{
    EXPECT_EQ(parse_verb("time.scale --factor abc"), "error");
    // 负值在 parser 层作为 double 被接受，范围钳制属控制器语义
    EXPECT_EQ(parse_verb("time.scale --factor -2"), "time.scale");
}

TEST(InputParserSemantic, FactorParsed)
{
    const Command c = InputParser{}.parse("time.scale --factor 2");
    EXPECT_EQ(c.verb, "time.scale");
    EXPECT_EQ(c.options.at("factor"), "2");
}

// CP-M-009 无参命令带多余位置参数：归入 remaining / 或报错，但不得误当必填
TEST(InputParserSemantic, ExtraPositionalOnNoArgCommand)
{
    const Command c = InputParser{}.parse("mine.stop extra 1 2");
    EXPECT_TRUE(c.verb == "mine.stop" || c.verb == "error")
        << "unexpected verb: " << c.verb;
}

// CP-M-010 跨 schema 互斥选项：mine.start 不接受 --factor
TEST(InputParserSemantic, CrossSchemaOptionRejected)
{
    EXPECT_EQ(parse_verb("mine.start --layer 1 --factor 2"), "error");
    EXPECT_EQ(parse_verb("time.now --layer 3"), "error");
}

// CP-M-011 数值前导补位：parser 保留字符串，值归一在业务层
TEST(InputParserSemantic, LeadingZerosKeptAsString)
{
    const Command c = InputParser{}.parse("mine.start --layer 0001");
    EXPECT_EQ(c.verb, "mine.start");
    EXPECT_EQ(c.options.at("layer"), "0001");
}

// CP-M-012 目标形态 `mine`（缺省层）在当前 schema 下应安全落空或报错
TEST(InputParserSemantic, BareMineNoLayer)
{
    const auto verb = parse_verb("mine");
    EXPECT_TRUE(verb == "error" || verb.empty()) << "unexpected verb: " << verb;
}

// CP-M-005/006 矿石资源存在性校验在语义/逻辑层由数据源完成，
// parser 侧仅保证将矿种映射到对应层时不崩溃（此处留回归锚点）。
TEST(InputParserSemantic, OreTargetsResolveWithoutCrash)
{
    // 需求示例：web空间形态，当前无 mine 子命令；仅论证不崩溃。
    EXPECT_NO_THROW({ (void)parse_verb("mine diamond"); });
    EXPECT_NO_THROW({ (void)parse_verb("mine copper"); });
    EXPECT_NO_THROW({ (void)parse_verb("mine iron"); });
}