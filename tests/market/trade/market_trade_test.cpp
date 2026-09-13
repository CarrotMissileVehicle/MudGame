/**
 * @file market_trade_test.cpp
 * @brief 集市买卖与日历黑盒测试。
 *
 * 覆盖：buy 扣款/金币不足拒绝/非法参数、sell 加钱与返回值、
 * 售价截断语义（DEF-106 锚点：0 售价不得加钱）、繁华集加价、日历推进。
 */
#include <gtest/gtest.h>

#include "Market.h"
#include "Shop.h"
#include "ShopItem.h"

namespace
{

struct SmithMarket
{
    Object hoe{"锄头", "工具", 10, 20, 50};   // 售 20 / 买 50
    Market market{};

    SmithMarket()
    {
        Shop shop("smith", "铁匠铺");
        shop.addItem(ShopItem(&hoe));
        market.registerShop(shop);
    }
};

} // namespace

TEST(MarketTrade, BuyChargesTotalPrice)
{
    SmithMarket f;
    long long gold = 200;
    EXPECT_TRUE(f.market.buy("smith", &f.hoe, 3, gold));
    EXPECT_EQ(gold, 200 - 50 * 3);
}

TEST(MarketTrade, BuyRejectsInsufficientGold)
{
    SmithMarket f;
    long long gold = 10;
    EXPECT_FALSE(f.market.buy("smith", &f.hoe, 1, gold));  // 50 > 10
    EXPECT_EQ(gold, 10);                                    // 金币不动
}

TEST(MarketTrade, BuyRejectsInvalidArgs)
{
    SmithMarket f;
    long long gold = 1000;
    EXPECT_FALSE(f.market.buy("no_such_shop", &f.hoe, 1, gold));
    EXPECT_FALSE(f.market.buy("smith", &f.hoe, 0, gold));
    EXPECT_FALSE(f.market.buy("smith", &f.hoe, -2, gold));
    EXPECT_FALSE(f.market.buy("smith", nullptr, 1, gold));
    EXPECT_EQ(gold, 1000);
}

TEST(MarketTrade, SellAddsGoldAndReturnsTotal)
{
    Market market;
    Object carrot{"胡萝卜", "蔬菜", 0, 10, 5};
    long long gold = 100;
    EXPECT_EQ(market.sell(&carrot, 4, gold), 40);
    EXPECT_EQ(gold, 140);
}

TEST(MarketTrade, SellRejectsInvalidArgs)
{
    Market market;
    Object carrot{"胡萝卜", "蔬菜", 0, 10, 5};
    long long gold = 100;
    EXPECT_EQ(market.sell(nullptr, 1, gold), 0);
    EXPECT_EQ(market.sell(&carrot, 0, gold), 0);
    EXPECT_EQ(market.sell(&carrot, -3, gold), 0);
    EXPECT_EQ(gold, 100);
}

TEST(MarketTrade, SellZeroPriceAddsNothing) // DEF-106 语义锚点
{
    Market market;
    Object junk{"废料", "杂物", 0, 0, 0};   // 售价 0
    long long gold = 5;
    EXPECT_EQ(market.sell(&junk, 2, gold), 0);
    EXPECT_EQ(gold, 5);
}

TEST(MarketTrade, ProsperousDayBoostsSellPrice)
{
    Object carrot{"胡萝卜", "蔬菜", 0, 10, 5};
    Market normal(1, 10);
    EXPECT_EQ(normal.getSellPrice(&carrot), 10);
    Market prosperous(3, 10);                 // 周三繁华集
    EXPECT_EQ(prosperous.getSellPrice(&carrot), 12);
}

TEST(MarketTrade, NullItemPricesAreZero)
{
    SmithMarket f;
    EXPECT_EQ(f.market.getSellPrice(nullptr), 0);
    EXPECT_EQ(f.market.getBuyPrice("smith", nullptr), 0);
    EXPECT_EQ(f.market.getBuyPrice("no_such_shop", &f.hoe), 0);
}

TEST(MarketCalendar, OnNewDayAdvancesCalendar)
{
    Market market(1, 1);
    EXPECT_TRUE(market.isFestival());          // 每月 1 日节日集
    market.onNewDay(3, 15);
    EXPECT_EQ(market.getDayOfWeek(), 3);
    EXPECT_EQ(market.getDayOfMonth(), 15);
    EXPECT_TRUE(market.isProsperousDay());     // 周三繁华集
    EXPECT_FALSE(market.isFestival());
}
