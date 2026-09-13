/**
 * @file GameSession.cpp
 * @brief GameSession 实现：按声明顺序装配全部子系统并构建 GameContext。
 *
 * 成员初始化顺序即声明顺序；GameContext 以聚合初始化引用各成员。
 * 商店货架添加、集市注册与游戏会话开启放于构造函数体。
 */
#include "GameSession.h"

GameSession::GameSession()
    : timeService_{mud::time::GameDateTime{0, 1, 1, 8, 0}},
      move_{player_},
      farm_{std::vector<FarmLand>{FarmLand(), FarmLand(), FarmLand(), FarmLand()}},
      farming_{&farm_},
      seeds_{{ {"cabbage", &cabbage_}, {"carrot", &carrot_}, {"tomato", &tomato_},
               {"pumpkin", &pumpkin_}, {"lingzhi", &lingzhi_} }},
      cropNames_{{ {&cabbage_, "小白菜"}, {&carrot_, "胡萝卜"}, {&tomato_, "番茄"},
                   {&pumpkin_, "南瓜"}, {&lingzhi_, "灵芝"} }},
      fishPool_{{ &crucian_, &grassCarp_, &perch_, &rainbowTrout_, &kingCrab_ }},
      fishNames_{{ {&crucian_, "小鲫鱼"}, {&grassCarp_, "草鱼"}, {&perch_, "鲈鱼"},
                   {&rainbowTrout_, "虹鳟鱼"}, {&kingCrab_, "帝王蟹"} }},
      fishing_{fishPool_, 0.3f},
      seedCabbage_{"小白菜种子", "种下后收获小白菜", 0, 8, 5},
      seedCarrot_{"胡萝卜种子", "种下后收获胡萝卜", 0, 10, 8},
      seedTomato_{"番茄种子", "种下后收获番茄", 0, 12, 10},
      seedPumpkin_{"南瓜种子", "种下后收获南瓜", 0, 15, 15},
      seedLingzhi_{"灵芝孢子", "种下后收获灵芝", 0, 20, 25},
      fertNormal_{"普通肥料", "生长周期减半", 0, 10, 15},
      fertAdvanced_{"高级肥料", "生长周期加速", 0, 25, 30},
      seedShop_{"seed", "种子商店"},
      groceryShop_{"grocery", "杂货铺"},
      blacksmithShop_{"blacksmith", "铁匠铺"},
      weather_{timeService_, farm_},
      miningController_{oreData_, timeService_, &miningEvents_, &tools_},
      miningHandler_{miningController_},
      outRenderer_{tuiState_},
      terminal_{outRenderer_},
      context_{game_, timeService_, player_, move_, farm_, farming_, seeds_, cropNames_,
               normalFert_, advancedFert_, fishPool_, fishNames_, fishing_, market_, gold_,
               weather_, tools_, oreTable_, miningController_, miningHandler_, tuiState_,
               terminal_}
{
    seedShop_.addItem(ShopItem(&seedCabbage_));
    seedShop_.addItem(ShopItem(&seedCarrot_));
    seedShop_.addItem(ShopItem(&seedTomato_));
    seedShop_.addItem(ShopItem(&seedPumpkin_));
    seedShop_.addItem(ShopItem(&seedLingzhi_));
    groceryShop_.addItem(ShopItem(&fertNormal_));
    groceryShop_.addItem(ShopItem(&fertAdvanced_));
    // 铁匠铺：位于集市，专营工具修复服务（修复按丢失的矿石/金币扣费，不由货架商品表达）
    market_.registerShop(seedShop_);
    market_.registerShop(groceryShop_);
    market_.registerShop(blacksmithShop_);
    market_.onNewDay(timeService_.now()); // 同步集市日历到当前游戏时间
    game_.startSession();
}

GameSession::~GameSession() = default;