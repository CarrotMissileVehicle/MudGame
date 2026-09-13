/**
 * @file market_controller_test.cpp
 * @brief 集市交易控制器测试（main.cpp 处理器下沉）。
 *
 * MarketController 承载 market.buy / market.sell 完整业务流程：
 * 商店/物品解析（名字或序号）、扣款入包、数量钳制、售价守卫（DEF-106）。
 * 行为与下沉前 main.cpp 处理器逐条对齐。
 */
#include <gtest/gtest.h>

#include "MarketController.h"
#include "Market.h"
#include "Shop.h"
#include "ShopItem.h"
#include "Bag.h"

namespace
{

struct TradeFixture
{
    Object hoe{"锄头", "工具", 10, 20, 50};      // 售 20 / 买 50
    Object seed{"白菜种", "种子", 0, 5, 3};     // 售 5 / 买 3
    Bag bag;
    Market market{};

    TradeFixture()
    {
        Shop shop("smith", "铁匠铺");
        shop.addItem(ShopItem(&hoe));
        shop.addItem(ShopItem(&seed));
        market.registerShop(shop);
    }
};

} // namespace

// ---- buy ----

TEST(MarketControllerBuy, SuccessDeductsGoldAndStocks)
{
    TradeFixture f;
    long long gold = 200;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "锄头", 3, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::Ok);
    EXPECT_EQ(r.item_name, "锄头");
    EXPECT_EQ(r.bought, 3);
    EXPECT_EQ(r.spent, 150);                 // 50 x 3
    EXPECT_EQ(gold, 50);                     // 扣款
    EXPECT_EQ(f.bag.CountObject("锄头"), 3); // 入包
}

TEST(MarketControllerBuy, ByIndexResolvesItem)
{
    TradeFixture f;
    long long gold = 10;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "1", 2, gold);   // 序号 1 → 白菜种

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::Ok);
    EXPECT_EQ(r.item_name, "白菜种");
    EXPECT_EQ(r.spent, 6);                            // 3 x 2
    EXPECT_EQ(f.bag.CountObject("白菜种"), 2);
}

TEST(MarketControllerBuy, UnknownShopRejected)
{
    TradeFixture f;
    long long gold = 100;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("no_such", "锄头", 1, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::NoSuchShop);
    EXPECT_EQ(gold, 100);
    EXPECT_EQ(f.bag.GetSize(), 0u);
}

TEST(MarketControllerBuy, UnknownItemByNameRejected)
{
    TradeFixture f;
    long long gold = 100;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "不存在的物品", 1, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::NoSuchItem);
    EXPECT_EQ(gold, 100);
}

TEST(MarketControllerBuy, IndexOutOfRangeRejected)
{
    TradeFixture f;
    long long gold = 100;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "99", 1, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::NoSuchItem);
    EXPECT_EQ(gold, 100);
}

TEST(MarketControllerBuy, InsufficientGoldKeepsBag)
{
    TradeFixture f;
    long long gold = 10;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "锄头", 1, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::BuyRejected);
    EXPECT_EQ(gold, 10);
    EXPECT_EQ(f.bag.GetSize(), 0u);
}

TEST(MarketControllerBuy, ZeroCountRejected)
{
    TradeFixture f;
    long long gold = 100;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.buy("smith", "锄头", 0, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::BuyRejected);
    EXPECT_EQ(gold, 100);
    EXPECT_EQ(f.bag.GetSize(), 0u);
}

TEST(MarketControllerBuy, LargeAmountUses64BitTotal)
{
    Object dear{"金像", "奢侈品", 0, 100, 2000000000};  // 买价 2e9
    Bag bag;
    Market market;
    Shop shop("lux", "奢侈品店");
    shop.addItem(ShopItem(&dear));
    market.registerShop(shop);
    MarketController ctl(market, bag);

    long long gold = 7000000000LL;   // 7e9 > 6e9
    const auto r = ctl.buy("lux", "金像", 3, gold);

    EXPECT_EQ(r.status, MarketController::BuyResult::Status::Ok);
    EXPECT_EQ(r.spent, 6000000000LL);   // 2e9 x 3，int 会回绕
    EXPECT_EQ(gold, 1000000000LL);
}

// ---- sell ----

TEST(MarketControllerSell, SuccessAddsGoldAndRemoves)
{
    TradeFixture f;
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    long long gold = 100;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("胡萝卜", 2, gold);

    EXPECT_EQ(r.status, MarketController::SellResult::Status::Ok);
    EXPECT_EQ(r.item_name, "胡萝卜");
    EXPECT_EQ(r.sold, 2);
    EXPECT_EQ(r.gained, 20);
    EXPECT_EQ(gold, 120);
    EXPECT_EQ(f.bag.CountObject("胡萝卜"), 0);  // 已移除
}

TEST(MarketControllerSell, ByIndexResolvesName)
{
    TradeFixture f;
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    long long gold = 0;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("0", 1, gold);   // 序号 0 → 胡萝卜

    EXPECT_EQ(r.status, MarketController::SellResult::Status::Ok);
    EXPECT_EQ(r.item_name, "胡萝卜");
    EXPECT_EQ(gold, 10);
}

TEST(MarketControllerSell, UnknownItemRejected)
{
    TradeFixture f;
    long long gold = 50;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("没有的东西", 1, gold);

    EXPECT_EQ(r.status, MarketController::SellResult::Status::NoSuchItem);
    EXPECT_EQ(gold, 50);
}

TEST(MarketControllerSell, IndexOutOfRangeRejected)
{
    TradeFixture f;
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    long long gold = 50;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("5", 1, gold);   // 背包仅 1 项

    EXPECT_EQ(r.status, MarketController::SellResult::Status::NoSuchItem);
    EXPECT_EQ(gold, 50);
    EXPECT_EQ(f.bag.CountObject("胡萝卜"), 1);
}

TEST(MarketControllerSell, CountClampedToOwned)
{
    TradeFixture f;
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    long long gold = 0;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("胡萝卜", 10, gold);   // 只有 1 个

    EXPECT_EQ(r.status, MarketController::SellResult::Status::Ok);
    EXPECT_EQ(r.sold, 1);
    EXPECT_EQ(gold, 10);
}

TEST(MarketControllerSell, ZeroPriceKeepsItem)   // DEF-106 语义
{
    TradeFixture f;
    f.bag.AddObject(new Object("废料", "杂物", 0, 0, 0));   // 售价 0
    f.bag.AddObject(new Object("废料", "杂物", 0, 0, 0));   // 堆叠 x2
    long long gold = 5;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("废料", 2, gold);

    EXPECT_EQ(r.status, MarketController::SellResult::Status::ZeroPrice);
    EXPECT_EQ(gold, 5);                          // 不加钱
    EXPECT_EQ(f.bag.CountObject("废料"), 2);     // 不移除
}

TEST(MarketControllerSell, ZeroCountRejected)
{
    TradeFixture f;
    f.bag.AddObject(new Object("胡萝卜", "蔬菜", 0, 10, 5));
    long long gold = 0;
    MarketController ctl(f.market, f.bag);

    const auto r = ctl.sell("胡萝卜", 0, gold);

    EXPECT_EQ(r.status, MarketController::SellResult::Status::NoSuchItem);
    EXPECT_EQ(gold, 0);
    EXPECT_EQ(f.bag.CountObject("胡萝卜"), 1);   // 物品不动
}
