// PlayerSerializer 存读档往返测试（含 DEF-002/DEF-007 回归）。
// 验证：金币 64 位不截断、背包堆叠物品往返保真、玩家属性与工具状态还原。
#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "PlayerSerializer.h"
#include "Player.h"
#include "Object.h"

namespace
{
    const char* kTestSave = "serializer_roundtrip_test.sav";

    void cleanup() { std::remove(kTestSave); }
}

// 往返：玩家属性 / 背包堆叠 / 金币 / 工具状态全部保真
TEST(SerializerRoundTrip, PreservesAllState)
{
    cleanup();
    Player saved(AtTown, Waiting, 66, 100, 120, 33, 77);
    saved.GetBag().AddObject(new Object("胡萝卜", "田里种的", 0, 8, 5));
    saved.GetBag().AddObject(new Object("帝王蟹", "深海捕获", 0, 500, 250));

    Game game;
    long long gold = 123456;
    mud::tool::ToolController tools;
    tools.use_tool(mud::tool::ToolId::Pickaxe);   // 磨损矿镐

    PlayerSerializer serializer;
    ASSERT_TRUE(serializer.Save(kTestSave, saved, game, gold, tools, 42));

    Player loaded(AtHome, Sleeping, 1, 1, 1, 1, 1);
    Game loadedGame;
    long long loadedGold = 0;
    mud::tool::ToolController loadedTools;
    std::int64_t totalMinutes = -1;
    ASSERT_TRUE(serializer.Load(kTestSave, loaded, loadedGame, loadedGold, loadedTools, totalMinutes));

    EXPECT_EQ(loaded.GetPosition(), AtTown);
    EXPECT_EQ(loaded.GetSatiety(), 66);
    EXPECT_EQ(loaded.GetMaxSatiety(), 100);
    EXPECT_EQ(loaded.GetFarmingExp(), 120);
    EXPECT_EQ(loaded.GetFishExp(), 33);
    EXPECT_EQ(loaded.GetMineExp(), 77);
    EXPECT_EQ(loadedGold, 123456);
    EXPECT_EQ(totalMinutes, 42);

    EXPECT_EQ(loaded.GetBag().CountObject("胡萝卜"), 1);
    EXPECT_EQ(loaded.GetBag().CountObject("帝王蟹"), 1);
    EXPECT_EQ(loaded.GetBag().GetSize(), 2u);
    EXPECT_EQ(loadedTools.durability(mud::tool::ToolId::Pickaxe), 19);

    cleanup();
}

// DEF-007 回归：64 位金币不截断（int 上限约 21.4 亿）
TEST(SerializerGold, LargeGoldSurvivesRoundTrip)
{
    cleanup();
    Player player;
    Game game;
    long long bigGold = 3'000'000'000LL;   // 30 亿，超 int 范围
    mud::tool::ToolController tools;

    PlayerSerializer serializer;
    ASSERT_TRUE(serializer.Save(kTestSave, player, game, bigGold, tools, 0));

    Player loaded;
    Game loadedGame;
    long long loadedGold = 0;
    mud::tool::ToolController loadedTools;
    std::int64_t total = -1;
    ASSERT_TRUE(serializer.Load(kTestSave, loaded, loadedGame, loadedGold, loadedTools, total));

    EXPECT_EQ(loadedGold, 3'000'000'000LL);
    cleanup();
}

// 堆叠数量段（第 6 段）往返保真
TEST(SerializerRoundTrip, StackedQuantitySurvives)
{
    cleanup();
    Player saved;
    auto* obj = new Object("黯铁矿", "修复材料", 0, 20, 10);
    obj->SetQuantity(7);
    saved.GetBag().AddUnique(obj);

    Game game;
    long long gold = 0;
    mud::tool::ToolController tools;

    PlayerSerializer serializer;
    ASSERT_TRUE(serializer.Save(kTestSave, saved, game, gold, tools, 0));

    Player loaded;
    Game loadedGame;
    long long loadedGold = 0;
    mud::tool::ToolController loadedTools;
    std::int64_t total = -1;
    ASSERT_TRUE(serializer.Load(kTestSave, loaded, loadedGame, loadedGold, loadedTools, total));

    EXPECT_EQ(loaded.GetBag().CountObject("黯铁矿"), 7);
    EXPECT_EQ(loaded.GetBag().GetSize(), 1u);
    cleanup();
}

// 读档不存在文件返回 false
TEST(SerializerLoad, MissingFileReturnsFalse)
{
    Player player;
    Game game;
    long long gold = 0;
    mud::tool::ToolController tools;
    std::int64_t total = -1;
    PlayerSerializer serializer;
    EXPECT_FALSE(serializer.Load("no_such_file_9x7.sav", player, game, gold, tools, total));
}

// DEF-002 回归：读档对已有物品的玩家整体赋值后无泄漏无崩溃（深拷贝语义）
TEST(SerializerRoundTrip, LoadOverwritesExistingBagSafely)
{
    cleanup();
    Player saved;
    saved.GetBag().AddObject(new Object("南瓜", "收获", 0, 15, 8));
    Game game;
    long long gold = 10;
    mud::tool::ToolController tools;

    PlayerSerializer serializer;
    ASSERT_TRUE(serializer.Save(kTestSave, saved, game, gold, tools, 0));

    // 目标玩家已有另一批物品：Load 内部整体赋值替换（旧物品经深拷贝赋值释放）
    Player target;
    target.GetBag().AddObject(new Object("旧物品A", "", 0, 1, 1));
    target.GetBag().AddObject(new Object("旧物品B", "", 0, 1, 1));

    Game loadedGame;
    long long loadedGold = 0;
    mud::tool::ToolController loadedTools;
    std::int64_t total = -1;
    ASSERT_TRUE(serializer.Load(kTestSave, target, loadedGame, loadedGold, loadedTools, total));

    EXPECT_EQ(target.GetBag().GetSize(), 1u);
    EXPECT_EQ(target.GetBag().CountObject("南瓜"), 1);
    EXPECT_FALSE(target.GetBag().HasObject("旧物品A"));
    cleanup();
}
