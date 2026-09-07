/**
 * @file probability_test.cpp
 * @brief 采矿掉落概率测试（方案 MIN-PB）：大样本统计校验产出分布与配置一致。
 *
 * 分布来源：src/Data/Ore/spawn_rates.json。
 *   shallow：ore_iron 0.70 / ore_silver 0.25 / ore_gold 0.05
 *   core   ：ore_iron 0.00（隔离校验：不应产出铁）
 * 判定：实测频率落在 [p-3σ, p+3σ]，σ=√(p(1-p)/n)。
 * 随机源为线程局部 mt19937（不可注入），故采用统计口径而非确定性断言。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

#include <cmath>
#include <map>
#include <string>

using namespace mud;

namespace
{

constexpr std::size_t kSamples = 200000; // 每层大样本量（栈/堆无关，仅计数）

// 自浅层(shallow)采集 n 次，返回各 ore_id 出现次数。
std::map<std::string, std::size_t> sample_shallow(std::size_t n, mining::MiningContext& ctx,
                                                  mud::TimeService& ts, MiningHandler& handler)
{
    std::map<std::string, std::size_t> counts;
    for (std::size_t i = 0; i < n; ++i)
    {
        ts.update(); // 1 帧 = 1 游戏分钟 = 1 interval → 每次恰好产出 1 份
        const auto results = handler.poll(ctx);
        for (const auto& r : results)
            if (!r.ore_id.empty()) ++counts[r.ore_id];
    }
    return counts;
}

// 4阶统计：给定样本比例与期望 p，计算 p±3σ 的合格下限/上限。
struct Tolerance { double lo; double hi; };
Tolerance tol(double p, std::size_t n)
{
    const double sigma = std::sqrt(p * (1.0 - p) / static_cast<double>(n));
    return {p - 3.0 * sigma, p + 3.0 * sigma};
}

} // namespace

// MIN-PB-002 大样本分布符合配置（基准比例）
TEST(MiningProbability, ShallowDistributionWithinTolerance)
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller(dummy_ore, ts);
    MiningHandler handler(controller);
    mining::MiningContext ctx;
    ctx.mining_level = 15;
    ctx.has_torch = true;
    ctx.has_lantern = true;
    ctx.tool.interval = 1;
    ASSERT_TRUE(handler.start(0, ctx)); // shallow

    const auto counts = sample_shallow(kSamples, ctx, ts, handler);
    const double n = static_cast<double>(kSamples);

    ASSERT_EQ(counts.size(), 3u); // shallow 仅有 iron/silver/gold
    const double iron   = static_cast<double>(counts.at("ore_iron")) / n;
    const double silver = static_cast<double>(counts.at("ore_silver")) / n;
    const double gold   = static_cast<double>(counts.at("ore_gold")) / n;

    const auto iron_t   = tol(0.70, kSamples);
    const auto silver_t = tol(0.25, kSamples);
    const auto gold_t   = tol(0.05, kSamples);

    EXPECT_GE(iron, iron_t.lo);
    EXPECT_LE(iron, iron_t.hi);
    EXPECT_GE(silver, silver_t.lo);
    EXPECT_LE(silver, silver_t.hi);
    EXPECT_GE(gold, gold_t.lo);
    EXPECT_LE(gold, gold_t.hi);
}

// MIN-PB-004 各档概率和守恒：样本比例之和≈100%
TEST(MiningProbability, ProbabilitySumsToOne)
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller(dummy_ore, ts);
    MiningHandler handler(controller);
    mining::MiningContext ctx;
    ctx.mining_level = 15;
    ctx.has_torch = true;
    ctx.has_lantern = true;
    ctx.tool.interval = 1;
    ASSERT_TRUE(handler.start(0, ctx));

    const auto counts = sample_shallow(kSamples, ctx, ts, handler);
    double sum = 0.0;
    for (const auto& [id, cnt] : counts) sum += static_cast<double>(cnt);
    EXPECT_NEAR(sum / static_cast<double>(kSamples), 1.0, 1e-3);
}

// MIN-PB-006 层隔离：core 层权重 0 的矿石（ore_iron）不应产出
TEST(MiningProbability, CoreLayerDoesNotDropIron)
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller(dummy_ore, ts);
    MiningHandler handler(controller);
    mining::MiningContext ctx;
    ctx.mining_level = 15;
    ctx.has_torch = true;
    ctx.has_lantern = true;
    ctx.tool.interval = 1;

    // core 层权重：iron 0.00 / silver 0.05 / gold 0.15 / crystal 0.30 / core 0.50
    ASSERT_TRUE(handler.start(4, ctx));
    for (std::size_t i = 0; i < kSamples; ++i)
    {
        ts.update();
        for (const auto& r : handler.poll(ctx))
        {
            EXPECT_NE(r.ore_id, "ore_iron") << "core 层不应产出 ore_iron";
        }
    }
}