/**
 * @file parse_double_test.cpp
 * @brief 数值参数校验助手测试（CP-M 系列）。
 *
 * parse_double 为命令处理器提供安全数值解析：失败返回 false
 * 且不修改 out，替代处理器内裸 try/catch std::stod（DEF-102 下沉）。
 */
#include <gtest/gtest.h>

#include "input_parser.h"

TEST(ParseDouble, ValidNumbers)
{
    double v = -1.0;
    EXPECT_TRUE(mud::cmd::parse_double("60", v));
    EXPECT_DOUBLE_EQ(v, 60.0);

    EXPECT_TRUE(mud::cmd::parse_double("0.5", v));
    EXPECT_DOUBLE_EQ(v, 0.5);

    EXPECT_TRUE(mud::cmd::parse_double("-3", v));
    EXPECT_DOUBLE_EQ(v, -3.0);

    EXPECT_TRUE(mud::cmd::parse_double("1e3", v));
    EXPECT_DOUBLE_EQ(v, 1000.0);

    // 容忍首尾空白（交互输入残留）
    EXPECT_TRUE(mud::cmd::parse_double(" 600 ", v));
    EXPECT_DOUBLE_EQ(v, 600.0);

    EXPECT_TRUE(mud::cmd::parse_double("600\r", v));
    EXPECT_DOUBLE_EQ(v, 600.0);
}

TEST(ParseDouble, RejectsGarbage)
{
    double v = 42.0;
    EXPECT_FALSE(mud::cmd::parse_double("", v));
    EXPECT_FALSE(mud::cmd::parse_double("   ", v));
    EXPECT_FALSE(mud::cmd::parse_double("abc", v));
    EXPECT_FALSE(mud::cmd::parse_double("6x", v));
    EXPECT_FALSE(mud::cmd::parse_double("1,5", v));
}

TEST(ParseDouble, RejectsNonFinite)
{
    double v = 42.0;
    EXPECT_FALSE(mud::cmd::parse_double("nan", v));
    EXPECT_FALSE(mud::cmd::parse_double("inf", v));
    EXPECT_FALSE(mud::cmd::parse_double("-inf", v));
}

TEST(ParseDouble, OutUntouchedOnFailure)
{
    double v = 42.0;
    EXPECT_FALSE(mud::cmd::parse_double("6x", v));
    EXPECT_DOUBLE_EQ(v, 42.0);
    EXPECT_FALSE(mud::cmd::parse_double("nan", v));
    EXPECT_DOUBLE_EQ(v, 42.0);
}
