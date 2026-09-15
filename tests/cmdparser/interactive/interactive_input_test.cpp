// 交互式参数收集测试：默认值 / 必填 / 取消（输入 q）。
// 验证 ParameterCollector.collect 在 StringRenderer 下的行为。
#include <gtest/gtest.h>

#include <string>

#include "input_parser.h"
#include "Renderer.h"

namespace
{
    // 提供预置输入序列的输入函数
    std::function<std::string()> make_input_fn(const std::vector<std::string>& inputs, std::size_t& idx)
    {
        return [&idx, inputs]() {
            if (idx >= inputs.size()) return std::string{};
            return inputs[idx++];
        };
    }
}

TEST(ParameterCollector, CollectsRequiredParameters)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"layer", "目标层(0-4)", true, ""});
    schema.parameters.push_back({"factor", "倍率", true, ""});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({"2", "1.5"}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "mine.start";

    ASSERT_TRUE(collector.collect(schema, cmd));
    EXPECT_EQ(cmd.options["layer"], "2");
    EXPECT_EQ(cmd.options["factor"], "1.5");
}

TEST(ParameterCollector, UsesDefaultWhenInputEmpty)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"count", "数量", true, "1"});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({""}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "market.buy";

    ASSERT_TRUE(collector.collect(schema, cmd));
    EXPECT_EQ(cmd.options["count"], "1");
}

TEST(ParameterCollector, CancelOnQ)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"shop", "商店ID", true, ""});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({"q"}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "market.buy";

    EXPECT_FALSE(collector.collect(schema, cmd));
}

TEST(ParameterCollector, RequiredMissingReturnsFalse)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"tool", "工具名", true, ""});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({""}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "blacksmith.repair";

    EXPECT_FALSE(collector.collect(schema, cmd));
}

TEST(ParameterCollector, SkipsPresentOptions)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"layer", "目标层(0-4)", true, ""});
    schema.parameters.push_back({"factor", "倍率", true, ""});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({"0.5"}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "mine.start";
    cmd.options["layer"] = "3";   // 已有值，跳过收集

    ASSERT_TRUE(collector.collect(schema, cmd));
    EXPECT_EQ(cmd.options["layer"], "3");
    EXPECT_EQ(cmd.options["factor"], "0.5");
}

// 第二个参数处取消：第一个参数不得残留在 cmd.options 中（一次性提交）
TEST(ParameterCollector, CancelMidCollectionLeavesCommandUnmodified)
{
    mud::cmd::CommandSchema schema;
    schema.parameters.push_back({"shop", "商店ID", true, ""});
    schema.parameters.push_back({"item", "物品名", true, ""});

    std::size_t idx = 0;
    mud::view::StringRenderer r;
    ParameterCollector collector(r, make_input_fn({"smith", "q"}, idx));

    mud::cmd::Command cmd;
    cmd.verb = "market.buy";
    cmd.options["pre_existing"] = "keep";

    EXPECT_FALSE(collector.collect(schema, cmd));
    EXPECT_EQ(cmd.options.count("shop"), 0u);          // 已输入但未提交
    EXPECT_EQ(cmd.options.count("item"), 0u);
    EXPECT_EQ(cmd.options["pre_existing"], "keep");    // 既有内容不受影响
}