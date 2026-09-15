/**
 * @file load_robustness_test.cpp
 * @brief 读档健壮性测试（DEF-104 / DEF-105）。
 *
 * 存档属不可信输入：损坏数值字段（stoi/stoll 抛异常）与非法日历字段
 * （月=0/13、日=32 等，sys_days{ymd} 为 UB）均不得令 Load 崩溃，
 * 而应回退默认值并保持调用方未损坏的初值。
 */
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "PlayerSerializer.h"
#include "Player.h"
#include "Game.h"
#include "tool_controller.h"

namespace
{

std::filesystem::path temp_save_path(const char* tag)
{
    return std::filesystem::temp_directory_path() /
           ("mudgame_robust_" + std::string(tag) + ".sav");
}

void write_save(const std::filesystem::path& path, const std::string& body)
{
    std::ofstream file(path.string());
    file << body;
}

} // namespace

TEST(LoadRobustness, CorruptedNumericsFallbackNoCrash) // DEF-104
{
    const auto path = temp_save_path("numeric");
    std::remove(path.string().c_str());
    write_save(path,
               "position=Town\nstate=Waiting\n"
               "satiety=abc\nmaxSatiety=99999999999\nfarmingExp=\n"
               "fishExp=0\nmineExp=0\ngold=99999999999999999999999\n"
               "saveOpenYear=2026\nsaveOpenMonth=1\nsaveOpenDay=1\n"
               "saveOpenHour=8\nsaveOpenMinute=30\ntotalPlaySeconds=0\n"
               "toolHoeLevel=1x2\ntoolHoeDurability=?!\n"
               "bagCount=0\n");

    PlayerSerializer ser;
    Player dst;
    Game dstGame;
    long long gold = 555;
    mud::tool::ToolController tools;
    std::int64_t dstMin = -1;

    EXPECT_EQ(ser.Load(path.string(), dst, dstGame, gold, tools, dstMin),
              PlayerSerializer::LoadStatus::Ok);
    EXPECT_EQ(dst.GetPosition(), AtTown);
    // 非法数值回退默认：饱食 100/100、经验 0
    EXPECT_EQ(dst.GetSatiety(), 100);
    EXPECT_EQ(dst.GetMaxSatiety(), 100);
    EXPECT_EQ(dst.GetFarmingExp(), 0);
    // 金币行损坏：视为无该段，保持调用方初值
    EXPECT_EQ(gold, 555);
    EXPECT_EQ(dstMin, -1);

    std::remove(path.string().c_str());
}

TEST(LoadRobustness, InvalidCalendarKeepsDefaultNoCrash) // DEF-105
{
    const auto path = temp_save_path("cal");
    std::remove(path.string().c_str());
    write_save(path,
               "position=Home\nstate=Waiting\nsatiety=80\nmaxSatiety=100\n"
               "farmingExp=0\nfishExp=0\nmineExp=0\ngold=1\n"
               "saveOpenYear=2026\nsaveOpenMonth=0\nsaveOpenDay=32\n"
               "saveOpenHour=12\nsaveOpenMinute=0\ntotalPlaySeconds=0\n"
               "bagCount=0\n");

    PlayerSerializer ser;
    Player dst;
    Game dstGame;
    long long gold = 0;
    mud::tool::ToolController tools;
    std::int64_t dstMin = -1;

    EXPECT_EQ(ser.Load(path.string(), dst, dstGame, gold, tools, dstMin),
              PlayerSerializer::LoadStatus::Ok);
    // 非法日历（月=0、日=32）：不设存档时间，保持默认（等价全新 Game）
    Game fresh;
    EXPECT_EQ(dstGame.getSaveOpenTime().total_minutes(),
              fresh.getSaveOpenTime().total_minutes());

    std::remove(path.string().c_str());
}

TEST(LoadRobustness, CorruptedItemFieldsFallbackNoCrash) // DEF-104
{
    const auto path = temp_save_path("item");
    std::remove(path.string().c_str());
    write_save(path,
               "position=Home\nstate=Waiting\nsatiety=100\nmaxSatiety=100\n"
               "farmingExp=0\nfishExp=0\nmineExp=0\ngold=10\n"
               "saveOpenYear=2026\nsaveOpenMonth=1\nsaveOpenDay=1\n"
               "saveOpenHour=0\nsaveOpenMinute=0\ntotalPlaySeconds=0\n"
               "bagCount=2\n"
               "item=黯铁矿|矿石|abc|20|0|3\n"
               "item=小麦|农作物|0|5x|3|zz\n");

    PlayerSerializer ser;
    Player dst;
    Game dstGame;
    long long gold = 0;
    mud::tool::ToolController tools;
    std::int64_t dstMin = -1;

    EXPECT_EQ(ser.Load(path.string(), dst, dstGame, gold, tools, dstMin),
              PlayerSerializer::LoadStatus::Ok);
    ASSERT_EQ(dst.GetBag().GetSize(), 2u);
    // 非法健康度/售价回退 0；非法数量回退 1；合法字段不受影响
    const auto& objs = dst.GetBag().GetObjects();
    EXPECT_EQ(objs[0]->GetName(), "黯铁矿");
    EXPECT_EQ(objs[0]->GetHealth(), 0);
    EXPECT_EQ(objs[0]->GetQuantity(), 3);
    // "5x" 走 stoi 前缀解析得 5（既有语义，不视为损坏）
    EXPECT_EQ(objs[1]->GetSellingPrice(), 5);
    // "zz" 完全非法 → 数量回退 1
    EXPECT_EQ(objs[1]->GetQuantity(), 1);

    std::remove(path.string().c_str());
}

TEST(LoadRobustness, MissingFileReportsNotFound) // DEF-385
{
    const auto path = temp_save_path("missing");
    std::remove(path.string().c_str()); // 确保文件不存在

    PlayerSerializer ser;
    Player dst;
    Game dstGame;
    long long gold = 42;
    mud::tool::ToolController tools;
    std::int64_t dstMin = -1;

    EXPECT_EQ(ser.Load(path.string(), dst, dstGame, gold, tools, dstMin),
              PlayerSerializer::LoadStatus::NotFound);
    // 失败时输出参数必须保持不变（commit-on-success）
    EXPECT_EQ(gold, 42);
    EXPECT_EQ(dstMin, -1);
    EXPECT_EQ(dst.GetSatiety(), 100);     // 默认 Player 初值：饱食 100，Load 未触碰
    EXPECT_EQ(dst.GetMaxSatiety(), 100);
}
