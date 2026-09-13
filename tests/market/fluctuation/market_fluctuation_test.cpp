/**
 * @file market_fluctuation_test.cpp
 * @brief 集市供需浮动测试（DEF-107）。
 *
 * 契约：updateFluctuations 每天选取 2-3 种「不同」在售商品浮动 ±30%，
 * 返回值即受影响品种数。修复前有放回抽取可命中同一商品——
 * map 覆盖导致实际品种数缩水、返回值虚报。
 */
#include <gtest/gtest.h>

#include <vector>

#include "Market.h"
#include "Shop.h"
#include "ShopItem.h"

namespace
{

struct FourItemMarket
{
    Object a{"白菜", "蔬菜", 0, 10, 5};
    Object b{"萝卜", "蔬菜", 0, 10, 5};
    Object c{"南瓜", "蔬菜", 0, 10, 5};
    Object d{"茄子", "蔬菜", 0, 10, 5};
    std::vector<Object*> all{&a, &b, &c, &d};
    Market market{};

    FourItemMarket()
    {
        Shop shop("general", "杂货铺");
        for (Object* o : all) shop.addItem(ShopItem(o));
        market.registerShop(shop);
    }

    int distinctFluctuating() const
    {
        int n = 0;
        for (const Object* o : all)
            if (market.getFluctuation(const_cast<Object*>(o)) != 1.0f) ++n;
        return n;
    }
};

} // namespace

TEST(MarketFluctuation, ClaimedCountEqualsDistinctItems) // DEF-107
{
    FourItemMarket f;
    // 50 天滚动：任一天返回值必须等于「实际浮动的不同品种数」。
    // 修复前有放回抽样下 50 天全巧合无重复的概率约 1e-13 → 必红。
    for (int day = 0; day < 50; ++day) {
        const int claimed = f.market.updateFluctuations();
        ASSERT_EQ(f.distinctFluctuating(), claimed)
            << "day=" << day << " 返回值虚报受影响品种数";
    }
}

TEST(MarketFluctuation, FactorStaysWithin30Percent)
{
    FourItemMarket f;
    f.market.updateFluctuations();
    for (const Object* o : f.all) {
        const float factor = f.market.getFluctuation(const_cast<Object*>(o));
        EXPECT_GE(factor, 0.7f - 1e-4f);
        EXPECT_LE(factor, 1.3f + 1e-4f);
    }
}

TEST(MarketFluctuation, NoShopsYieldsZero)
{
    Market market;
    EXPECT_EQ(market.updateFluctuations(), 0);
    Object orphan{"孤品", "无商店", 0, 10, 5};
    EXPECT_EQ(market.getFluctuation(&orphan), 1.0f);
    EXPECT_EQ(market.getFluctuation(nullptr), 1.0f);
}
