/**
 * @file player_serializer_roundtrip_test.cpp
 * @brief 存档/读档往返测试。
 *
 * 覆盖：完整字段（玩家/金币/游戏时钟/工具/背包堆叠）Save→Load 还原、
 * Load 深拷贝替换玩家实例（原子对象指针不再共享）、
 * 无 gold/工具段的旧存档兼容（保持调用方初值）。
 */
#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "PlayerSerializer.h"
#include "Player.h"
#include "Game.h"
#include "tool_controller.h"

using namespace std::chrono;

namespace {

// 每个用例独立临时存档路径
std::filesystem::path temp_save_path(const char* tag)
{
    return std::filesystem::temp_directory_path() / ("mudgame_roundtrip_" + std::string(tag) + ".sav");
}

} // namespace

TEST(PlayerSerializerRoundTrip, FullFieldsSurvive)
{
    const auto path = temp_save_path("full");
    std::remove(path.string().c_str());

    Player src(AtTown, Shopping, 66, 100, 15, 4, 7);
    src.GetBag().AddObject(new Object("黯铁矿", "矿石", 0, 20, 0));
    src.GetBag().AddObject(new Object("黯铁矿", "矿石", 0, 20, 0)); // 应合并为同一堆叠 x2

    Game game;
    game.setTotalPlayTime(seconds{1234});

    mud::tool::ToolController tools;
    tools.restore(mud::tool::ToolId::Hoe, 3, 25);
    tools.restore(mud::tool::ToolId::Rod, 1, 5);

    PlayerSerializer ser;
    ASSERT_TRUE(ser.Save(path.string(), src, game, 321, tools, 7 * 24 * 60 + 45));

    Player dst;
    Game dstGame;
    int dstGold = 0;
    mud::tool::ToolController dstTools;
    std::int64_t dstMin = -1;
    ASSERT_TRUE(ser.Load(path.string(), dst, dstGame, dstGold, dstTools, dstMin));

    EXPECT_EQ(dst.GetPosition(), AtTown);
    EXPECT_EQ(dst.GetState(), Shopping);
    EXPECT_EQ(dst.GetSatiety(), 66);
    EXPECT_EQ(dst.GetMaxSatiety(), 100);
    EXPECT_EQ(dst.GetFarmingExp(), 15);
    EXPECT_EQ(dst.GetFishExp(), 4);
    EXPECT_EQ(dst.GetMineExp(), 7);

    EXPECT_EQ(dstGold, 321);
    EXPECT_EQ(dstMin, 7 * 24 * 60 + 45);
    EXPECT_EQ(dstGame.getTotalPlayTime().count(), 1234);
    // SaveOpenTime 往返后分粒度一致（序列化走 GameDateTime 字段，天然丢弃秒）
    EXPECT_EQ(dstGame.getSaveOpenTime().total_minutes(),
              game.getSaveOpenTime().total_minutes());

    // 工具等级/耐久还原
    EXPECT_EQ(dstTools.level(mud::tool::ToolId::Hoe), 3);
    EXPECT_EQ(dstTools.durability(mud::tool::ToolId::Hoe), 25);
    EXPECT_EQ(dstTools.level(mud::tool::ToolId::Rod), 1);
    EXPECT_EQ(dstTools.durability(mud::tool::ToolId::Rod), 5);

    // 背包堆叠还原：同一堆叠 x2（AddUnique 不拆分成两条）
    EXPECT_EQ(dst.GetBag().CountObject("黯铁矿"), 2);
    EXPECT_EQ(dst.GetBag().GetSize(), 1u);
    EXPECT_EQ(dst.GetBag().GetObjects()[0]->GetQuantity(), 2);

    std::remove(path.string().c_str());
}

TEST(PlayerSerializerRoundTrip, LoadReplacesBagContentsWithoutDoubleFree)
{
    const auto path = temp_save_path("replace");
    std::remove(path.string().c_str());

    Player src;
    src.GetBag().AddObject(new Object("小麦", "农作物", 0, 5, 3));
    PlayerSerializer ser;
    ASSERT_TRUE(ser.Save(path.string(), src, Game{}, 0, mud::tool::ToolController{}, 0));

    Player dst(AtMine, Mining, 100, 100, 0, 0, 0);
    dst.GetBag().AddObject(new Object("旧物", "占位", 0, 1, 1)); // 读档前旧背包内容
    Game dstGame2;
    int dstGold2 = 0;
    mud::tool::ToolController dstTools2;
    std::int64_t dstMin2 = -1;
    ASSERT_TRUE(ser.Load(path.string(), dst, dstGame2, dstGold2, dstTools2, dstMin2));

    // 旧背包内容被存档内容替换（而非追加共享指针）
    ASSERT_EQ(dst.GetBag().GetSize(), 1u);
    EXPECT_EQ(dst.GetBag().GetObjects()[0]->GetName(), "小麦");
    EXPECT_FALSE(dst.GetBag().HasObject("旧物"));  // 旧对象已随深拷贝释放
    EXPECT_EQ(dst.GetBag().CountObject("小麦"), 1);

    std::remove(path.string().c_str());
}

TEST(PlayerSerializerRoundTrip, LegacySaveKeepsCallerDefaults)
{
    const auto path = temp_save_path("legacy");
    std::remove(path.string().c_str());

    // 手工构造旧格式存档：无 gold、无工具段
    {
        std::ofstream file(path.string());
        file << "# 旧存档\n";
        file << "position=Town\nstate=Waiting\nsatiety=50\nmaxSatiety=100\n";
        file << "farmingExp=0\nfishExp=0\nmineExp=0\n";
        file << "saveOpenYear=0\nsaveOpenMonth=1\nsaveOpenDay=1\n";
        file << "saveOpenHour=8\nsaveOpenMinute=0\ntotalPlaySeconds=0\n";
        file << "bagCount=0\n";
    }

    int gold = 777;
    mud::tool::ToolController tools;
    tools.use_tool(mud::tool::ToolId::Pickaxe); // 调用方已有耐久状态
    const int intactDura = tools.durability(mud::tool::ToolId::Pickaxe);

    PlayerSerializer ser;
    Player dst;
    Game dstGame;
    std::int64_t dstMin = -1;
    ASSERT_TRUE(ser.Load(path.string(), dst, dstGame, gold, tools, dstMin));

    EXPECT_EQ(gold, 777);                     // 无 gold 段：保持调用方
    EXPECT_EQ(tools.durability(mud::tool::ToolId::Pickaxe), intactDura); // 无工具段：不覆盖
    EXPECT_EQ(dst.GetPosition(), AtTown);
    EXPECT_EQ(dst.GetSatiety(), 50);

    std::remove(path.string().c_str());
}