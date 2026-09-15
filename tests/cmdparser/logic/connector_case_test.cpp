/**
 * @file connector_case_test.cpp
 * @brief Connector 大小写契约测试：has()/has_schema()/get_schema()
 *       与 bind()/dispatch() 一致地大小写不敏感；get_schema 未注册抛异常。
 */
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "connector.h"
#include "input_parser.h"

namespace
{
using mud::cmd::Command;
using mud::cmd::CommandSchema;
} // namespace

TEST(ConnectorCase, HasIsCaseInsensitive)
{
    Connector conn;
    conn.bind("mine.dig", [](const Command&, const HandlerContext&) {
        return HandlerResult::Ok;
    });

    EXPECT_TRUE(conn.has("mine.dig"));
    EXPECT_TRUE(conn.has("Mine.Dig"));
    EXPECT_TRUE(conn.has("MINE.DIG"));
    EXPECT_FALSE(conn.has("mine.digg"));
    EXPECT_FALSE(conn.has("other.verb"));
}

TEST(ConnectorCase, HasSchemaIsCaseInsensitive)
{
    Connector conn;
    conn.register_schema("mine.dig", CommandSchema{});
    conn.register_schema("farm.sow", CommandSchema{});

    EXPECT_TRUE(conn.has_schema("MINE.DIG"));
    EXPECT_TRUE(conn.has_schema("Farm.Sow"));
    EXPECT_FALSE(conn.has_schema("mine.digg"));
    EXPECT_FALSE(conn.has_schema("unknown.verb"));
}

TEST(ConnectorCase, GetSchemaCaseInsensitive)
{
    Connector conn;
    CommandSchema schema;
    schema.description = "挖矿";
    conn.register_schema("mine.dig", schema);

    EXPECT_EQ(conn.get_schema("Mine.Dig").description, "挖矿");
}

TEST(ConnectorCase, GetSchemaThrowsOutOfRangeForUnknownVerb)
{
    Connector conn;
    conn.register_schema("mine.dig", CommandSchema{});
    EXPECT_THROW(conn.get_schema("unknown.verb"), std::out_of_range);
}
