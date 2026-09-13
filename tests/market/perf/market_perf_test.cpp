/**
 * @file market_perf_test.cpp
 * @brief 集市性能烟囱测试：千日浮动刷新 + 全量报价查询。
 *
 * 预算取实际耗时约 500 倍余量，目的在于拦截复杂度级回退
 * （如误引入平方级扫描），而非精确基准测量。
 */
#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

#include "Market.h"
#include "Shop.h"
#include "ShopItem.h"

TEST(MarketPerf, ThousandDaysRefreshUnderBudget)
{
    Market market;
    std::vector<Object> items;
    items.reserve(500); // 预留避免扩容令指针失效
    for (int s = 0; s < 10; ++s) {
        Shop shop("shop" + std::to_string(s), "商店" + std::to_string(s));
        for (int i = 0; i < 50; ++i) {
            items.emplace_back("货品" + std::to_string(s * 50 + i), "杂货", 0, 10, 5);
            shop.addItem(ShopItem(&items.back()));
        }
        market.registerShop(shop);
    }
    ASSERT_EQ(market.shopCount(), 10u);

    const auto t0 = std::chrono::steady_clock::now();
    for (int day = 1; day <= 1000; ++day)
        market.onNewDay(day % 7 + 1, day % 30 + 1);
    long long sink = 0;
    for (auto& o : items) sink += market.getSellPrice(&o);
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    EXPECT_GT(sink, 0);
    EXPECT_LT(elapsed, 5.0); // 实际约 10ms 量级，500 倍余量防偶发
}
