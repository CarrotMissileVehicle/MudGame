# 悠闲农场 - 子系统设计文档

基于 `proj.md` v1.2 拆分，共识别 **12 个子系统**。

---

## 目录

1. [时间系统](#1-时间系统)
2. [玩家属性系统](#2-玩家属性系统)
3. [玩家状态机](#3-玩家状态机)
4. [地图系统](#4-地图系统)
5. [种菜系统](#5-种菜系统)
6. [钓鱼系统](#6-钓鱼系统)
7. [采矿系统](#7-采矿系统)
8. [集市系统](#8-集市系统)
9. [工具系统](#9-工具系统)
10. [天气与事件系统](#10-天气与事件系统)
11. 解析系统
12. [持久化系统](#12-持久化系统)

---

## 1. 时间系统

**职责：** 管理游戏内时间流逝、昼夜循环、倍速控制、时间节点通知。

### 数据结构

```
TimeSystem
├── game_hour: int          // 当前游戏小时 0-23
├── game_minute: int        // 当前游戏分钟 0-59
├── day_count: int          // 天数计数
├── speed_multiplier: int   // 倍速（固定 60）
└── is_daytime: bool        // 是否白天（06:00-18:00）
```

### 核心逻辑

| 功能 | 说明 |
|------|------|
| tick() | 每现实秒调用，game_minute += speed_multiplier |
| 进位处理 | minute >= 60 → hour++, minute = 0; hour >= 24 → day++, hour = 0 |
| isDaytime() | 06:00-18:00 返回 true，否则 false |
| getTimeNotifications() | 返回当前时刻需要触发的通知列表 |

### 时间节点通知

| 游戏时间 | 通知 ID | 内容 |
|---------|---------|------|
| 06:00 | TIME_DAWN | "天亮了，新的一天开始" |
| 08:00 | TIME_MORNING | "早上好" |
| 12:00 | TIME_NOON | "中午了" |
| 18:00 | TIME_DUSK | "天黑了，你感到疲倦" |
| 00:00 | TIME_MIDNIGHT | "深夜了" |

### 依赖

- 无外部依赖，被所有需要时间信息的系统引用

---

## 2. 玩家属性系统

**职责：** 管理玩家核心属性（饱食度、金币、三套经验值/等级）。

### 数据结构

```
Player
├── name: string
├── gold: int                    // 初始 50
├── satiety: int                 // 当前饱食度 0-100
├── max_satiety: int             // 固定 100
├── current_location: LocationId
│
├── farming_exp: int             // 农业经验
├── farming_level: int           // 农业等级
├── fishing_exp: int             // 钓鱼经验
├── fishing_level: int           // 钓鱼等级
├── mining_exp: int              // 采矿经验
└── mining_level: int            // 采矿等级（≤ 农业等级）
```

### 等级经验表（三套系统共用）

| 等级 | 所需经验 |
|------|---------|
| 1 | 0 |
| 2 | 100 |
| 3 | 300 |
| 5 | 800 |
| 8 | 2000 |
| 10 | 5000 |
| 15 | 12000 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| addExp(type, amount) | 增加经验值，自动计算升级 |
| checkLevelUp(type) | 检查是否达到下一等级阈值 |
| updateSatiety(delta) | 修改饱食度（衰减/进食） |
| canAct() | 饱食度 > 0 时返回 true |
| isSlowed() | 饱食度 < 30 时返回 true（速度减半） |
| addGold(amount) | 增加金币 |
| spendGold(amount) | 消耗金币，不足时返回 false |

### 依赖

- 等级表配置数据（data/level_table）

---

## 3. 玩家状态机

**职责：** 管理玩家当前行为状态，控制可执行操作的范围。

### 状态定义

```
enum PlayerState {
    IDLE,       // 行走中/空闲，可移动到任何地点
    FARMING,    // 种田状态（播种、浇水、施肥、收获）
    FISHING,    // 钓鱼状态（挂机产出）
    MINING      // 采矿状态（挂机产出）
}
```

### 状态转移图

```
IDLE ──移动到农田──► FARMING ──停止──► IDLE
IDLE ──移动到海岸──► FISHING ──停止──► IDLE
IDLE ──移动到矿洞──► MINING  ──停止──► IDLE
```

### 核心逻辑

| 功能 | 说明 |
|------|------|
| transition(newState) | 切换状态，校验是否允许转移 |
| canTransitionTo(newState) | IDLE 可转到任何活动状态；活动状态只能回到 IDLE |
| getAvailableActions() | 根据当前状态返回可执行操作列表 |
| isBusy() | 当前是否处于活动状态（FARMING/FISHING/MINING） |

### 地点与状态对应

| 地点 | 允许的状态 |
|------|-----------|
| 小屋（中央） | IDLE |
| 农田（左） | IDLE → FARMING |
| 海岸（右） | IDLE → FISHING |
| 小镇（上） | IDLE |
| 矿洞（下） | IDLE → MINING |

### 依赖

- 地图系统（判断目标地点是否可达）
- 玩家属性系统（饱食度判断）

---

## 4. 地图系统

**职责：** 管理地图布局、地点定义、解锁条件、地点间移动。

### 地图布局（十字型）

```
            [小镇]
              |
   [农田] - [小屋] - [海岸]
              |
            [矿洞]
```

### 地点定义

```
enum LocationId {
    HOME,           // 小屋（中央）
    FARM,           // 农田（左）
    COAST,          // 海岸（右）
    TOWN,           // 小镇（上）
    MINE_SHALLOW,   // 矿洞·浅层（下）
    MINE_MIDDLE,    // 矿洞·中层
    MINE_DEEP,      // 矿洞·深层
    MINE_CRYSTAL,   // 矿洞·水晶层
    MINE_CORE       // 矿洞·地心
}
```

### 地点解锁条件

| 地点 | 解锁条件 | 解锁类型 |
|------|---------|---------|
| HOME | 初始 | — |
| FARM | 初始 | — |
| COAST | 初始 | — |
| TOWN | 初始 | — |
| MINE_SHALLOW | 农业等级 ≥ 1 | 等级 |
| MINE_MIDDLE | 农业等级 ≥ 3 | 等级 |
| MINE_DEEP | 农业等级 ≥ 5 | 等级 |
| MINE_CRYSTAL | 农业等级 ≥ 8 | 等级 |
| MINE_CORE | 农业等级 ≥ 15 | 等级 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| canMoveTo(locationId) | 检查目标地点是否已解锁 |
| move(player, targetLocation) | 移动到目标地点，切换玩家状态为 IDLE |
| getUnlockedLocations(player) | 返回所有已解锁地点列表 |
| getAvailableActions(locationId) | 返回该地点可执行的行为列表 |

### 地点行为表

| 地点 | 可执行行为 |
|------|-----------|
| HOME | 进食、睡觉、查看背包、查看状态 |
| FARM | 播种、浇水、施肥、收获 |
| COAST | 钓鱼 |
| TOWN | 集市交易、铁匠铺修复/升级、商店购买 |
| MINE_* | 采矿（挂机模式） |

### 依赖

- 玩家属性系统（读取等级判断解锁）

---

## 5. 种菜系统

**职责：** 管理农田地块、作物种植、生长、浇水、施肥、收获。

### 数据结构

```
Farm
├── level: int                     // 农田等级，决定地块数量
├── capacity: int                  // 当前地块数（level × 初始值）
└── plots: Plot[]                  // 地块列表

Plot
├── crop_id: string | null         // 种植的作物 ID
├── growth_stage: int              // 当前生长阶段
├── growth_max: int                // 最大生长阶段
├── watered: bool                  // 是否已浇水
└── fertilized: bool               // 是否已施肥
```

### 作物配置（5 种）

| ID | 作物 | 类型 | 种子价格 | 生长天数 | 收获数量 | 售价 | 农业经验 | 解锁 |
|----|------|------|---------|---------|---------|------|---------|------|
| cabbage | 小白菜 | 经验 | 5 | 2 | 2 | 8/个 | +12 | 初始 |
| carrot | 胡萝卜 | 经济 | 8 | 3 | 2 | 12/个 | +8 | 初始 |
| tomato | 番茄 | 经济 | 15 | 4 | 3 | 10/个 | +8 | 农业 Lv.2 |
| pumpkin | 南瓜 | 经济 | 25 | 5 | 1 | 40/个 | +15 | 农业 Lv.3 |
| lingzhi | 灵芝草 | 经验 | 60 | 6 | 1 | 40/个 | +50 | 农业 Lv.10 |

### 肥料配置

| ID | 肥料 | 价格 | 效果 |
|----|------|------|------|
| fertilizer_normal | 普通肥料 | 10 | 剩余生长周期减半 |
| fertilizer_super | 高级肥料 | 30 | 剩余生长周期减为 1/3 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| plant(plotIndex, cropId) | 在指定地块播种，消耗种子 |
| waterAll() | 一键浇水所有已种植地块，消耗饱食度 = 地块数 × 5 |
| fertilize(plotIndex, fertilizerId) | 对指定地块施肥，剩余周期 × 0.5 或 × 0.33 |
| harvest(plotIndex) | 收获作物，获得物品 + 农业经验 |
| growAll(isDaytime, weather) | 每 tick 调用，推进所有作物生长；非白天或饱食度 < 30 时速度减半 |
| getPlotStatus() | 返回所有地块状态摘要 |
| upgradeFarm() | 升级农田，增加地块数量 |

### 天气影响

| 天气 | 效果 |
|------|------|
| 小雨 | 自动浇水所有地块 |
| 干旱 | 作物减产 30% |

### 依赖

- 时间系统（判断昼夜，影响生长速度）
- 玩家属性系统（消耗饱食度，判断是否减半）
- 背包系统（消耗种子，存入收获物）
- 天气系统（读取当日天气）

---

## 6. 钓鱼系统

**职责：** 管理钓鱼挂机产出、鱼类概率、鱼饵效果。

### 数据结构

```
FishingSession
├── is_active: bool
├── bait_id: string | null          // 当前使用的鱼饵
├── rod_level: int                  // 鱼竿等级
└── last_catch_time: timestamp      // 上次产出时间

FishData
├── id: string
├── name: string
├── spawn_rate: float               // 基础出现概率
├── sell_price: int
├── fishing_exp: int
└── special_effect: string | null   // 特殊效果
```

### 鱼类配置（5 种）

| ID | 鱼类 | 概率 | 售价 | 钓鱼经验 | 特殊效果 |
|----|------|------|------|---------|---------|
| crucian | 小鲫鱼 | 40% | 10 | +5 | 恢复饱食度 +15 |
| grass | 草鱼 | 30% | 15 | +8 | — |
| perch | 鲈鱼 | 20% | 25 | +12 | — |
| rainbow | 彩虹鳟鱼 | 8% | 50 | +20 | 稀有料理食材 |
| crab | 帝王蟹 | 2% | 200 | +40 | 高价收购品 |

### 鱼饵配置

| ID | 鱼饵 | 价格 | 效果 |
|----|------|------|------|
| bait_worm | 蚯蚓 | 5 | 普通鱼概率 +10% |
| bait_dough | 面团 | 8 | 稀有鱼概率 +5% |
| bait_special | 特殊饵 | 20 | 稀有鱼概率 +15% |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| startFishing(baitId) | 开始钓鱼挂机 |
| stopFishing() | 停止钓鱼，结算收获 |
| tick(isDaytime) | 挂机期间每 tick 判断是否产出鱼；非白天速度减半 |
| rollFish(rodLevel, baitId) | 根据概率表 + 鱼竿等级加成 + 鱼饵加成计算产出 |
| getCatchSummary() | 返回本次钓鱼获得的所有鱼类统计 |

### 鱼竿等级加成

每升一级，稀有鱼基础概率 +2%。

### 依赖

- 时间系统（判断昼夜）
- 玩家属性系统（消耗饱食度）
- 背包系统（存入钓到的鱼）
- 工具系统（读取鱼竿等级和耐久）

---

## 7. 采矿系统

**职责：** 管理采矿挂机产出、矿石概率、矿区解锁。

### 数据结构

```
MiningSession
├── is_active: bool
├── mine_layer: MineLayerId        // 当前矿区
├── pickaxe_level: int             // 矿镐等级
├── has_light: bool                // 是否有照明（深层+需要）
└── last_mine_time: timestamp      // 上次产出时间

OreData
├── id: string
├── name: string
├── min_layer: int                 // 最早出现的矿区等级
├── sell_price: int
├── mining_exp: int
└──用途: string
```

### 矿石配置（5 种）

| ID | 矿石 | 最早矿区 | 售价 | 采矿经验 | 用途 |
|----|------|---------|------|---------|------|
| ore_iron | 黯铁矿 | 浅层(Lv.1) | 8 | +5 | 工具升级材料 |
| ore_silver | 星纹银 | 中层(Lv.3) | 25 | +12 | 高级工具材料 |
| ore_gold | 赤耀金 | 深层(Lv.5) | 60 | +20 | 贵重品/升级材料 |
| ore_crystal | 雷鸣晶 | 水晶层(Lv.8) | 150 | +35 | 传说工具材料 |
| ore_core | 世界之核 | 地心(Lv.15) | 500 | +60 | 至宝/收藏 |

### 矿石出现概率

| 矿区 | 黯铁矿 | 星纹银 | 赤耀金 | 雷鸣晶 | 世界之核 |
|------|--------|--------|--------|--------|---------|
| 浅层 | 70% | 25% | 5% | — | — |
| 中层 | 40% | 40% | 15% | 5% | — |
| 深层 | 15% | 35% | 35% | 13% | 2% |
| 水晶层 | 5% | 15% | 30% | 40% | 10% |
| 地心 | — | 5% | 15% | 30% | 50% |

### 矿区照明需求

| 矿区 | 需要照明 |
|------|---------|
| 浅层 | 否 |
| 中层 | 否 |
| 深层 | 是（火把） |
| 水晶层 | 是（灯笼） |
| 地心 | 是（灯笼） |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| startMining(layerId) | 开始采矿挂机 |
| stopMining() | 停止采矿，结算收获 |
| tick(isDaytime, satiety) | 挂机期间每 tick 判断是否产出矿石 |
| rollOre(pickaxeLevel, layerId) | 根据概率表 + 矿镐等级加成计算产出 |
| getMiningSummary() | 返回本次采矿获得的所有矿石统计 |
| canAccessLayer(layerId, player) | 检查是否满足矿区解锁条件 + 照明需求 |

### 矿镐等级加成

每升一级，稀有矿石基础概率 +3%，产出间隔 -0.5 秒。

### 依赖

- 时间系统（判断昼夜）
- 玩家属性系统（消耗饱食度，判断等级解锁）
- 背包系统（存入矿石）
- 工具系统（读取矿镐等级和耐久）

---

## 8. 集市系统

**职责：** 管理商店交易、供需波动、好感度、集市日历。

### 数据结构

```
Market
├── day_of_week: int               // 1-7（周一-周日）
├── day_of_month: int              // 1-30
├── price_fluctuations: Map<ItemId, float>  // 当日价格浮动
└── shop_favor: Map<ShopId, int>   // 各商店好感度

Shop
├── id: string
├── name: string
├── items: ShopItem[]              // 售卖物品列表
└── favor_level: int               // 好感度等级

ShopItem
├── item_id: string
├── base_price: int
├── current_price: int             // 基础价 × 浮动系数
└── unlock_favor: int              // 解锁所需好感度
```

### 商店定义

| ID | 名称 | 主营 |
|----|------|------|
| shop_seed | 种子铺 | 种子、肥料 |
| shop_fish | 渔具店 | 鱼竿、鱼饵 |
| shop_mine | 矿具店 | 矿镐、火把 |
| shop_grocery | 杂货铺 | 食物 |
| shop_blacksmith | 铁匠铺 | 工具升级、工具修复 |

### 集市日历

| 条件 | 效果 |
|------|------|
| 每日 | 正常价格 |
| 每周三 | 所有物品售价 +20% |
| 每月 1 日 | 节日集：稀有商品上架，限定事件触发 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| buy(player, shopId, itemId, count) | 购买物品 |
| sell(player, itemId, count) | 出售背包物品 |
| getSellPrice(itemId) | 获取当日售价（含浮动） |
| getBuyPrice(shopId, itemId) | 获取当日购买价（含浮动 + 好感折扣） |
| updateFluctuations() | 每天随机生成 2-3 种商品价格浮动 ±30% |
| addFavor(shopId, amount) | 交易增加好感度 |
| isFestival() | 是否节日集 |
| isProsperousDay() | 是否繁华集（周三） |

### 依赖

- 玩家属性系统（金币增减）
- 背包系统（物品增删）
- 工具系统（读取/修改工具耐久和等级）
- 时间系统（读取星期/日期）

---

## 9. 工具系统

**职责：** 管理工具的等级、耐久度、升级、修复。

### 数据结构

```
Tool
├── id: string
├── name: string
├── level: int                     // 当前等级
├── max_level: int                 // 最高等级
├── durability: int                // 当前耐久
├── max_durability: int            // 最大耐久
├── durability_per_use: int        // 每次使用消耗
└── is_broken: bool                // 耐久 ≤ 1 时为 true
```

### 工具定义

| ID | 名称 | 初始耐久 | 每次消耗 | 最高等级 |
|----|------|---------|---------|---------|
| tool_hoe | 锄头 | 50 | -1 | 3 |
| tool_rod | 鱼竿 | 30 | -1 | 3 |
| tool_pickaxe | 矿镐 | 25 | -2 | 3 |

### 升级配置

| 工具 | Lv.1→2 | Lv.2→3 |
|------|--------|--------|
| 锄头 | 50金 + 黯铁矿×5 | 100金 + 星纹银×3 + 赤耀金×1 |
| 鱼竿 | 80金 + 黯铁矿×5 | 200金 + 星纹银×3 + 赤耀金×1 |
| 矿镐 | 80金 + 黯铁矿×5 | 200金 + 星纹银×3 + 赤耀金×1 |

### 修复配置

| 工具 | 修复费用公式 | 矿石修复（费用减半） |
|------|------------|-------------------|
| 锄头 | 50 × (1 - durability/max_durability) | 黯铁矿×2 |
| 鱼竿 | 80 × (1 - durability/max_durability) | 星纹银×2 |
| 矿镐 | 80 × (1 - durability/max_durability) | 黯铁矿×2 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| use(toolId) | 使用工具，扣减耐久；耐久 ≤ 1 时返回 false |
| isBroken(toolId) | 检查工具是否损坏 |
| upgrade(toolId, player) | 升级工具，消耗金币和材料 |
| repair(toolId, player, useOre) | 修复工具，可选金币修复或矿石修复 |
| getLevelBonus(toolId) | 获取当前等级加成效果 |

### 升级效果

| 工具 | 升级效果 |
|------|---------|
| 锄头 | 每级 +1 地块，降低饱食度消耗 |
| 鱼竿 | 每级稀有鱼概率 +2% |
| 矿镐 | 每级稀有矿概率 +3%，产出间隔 -0.5 秒 |

### 依赖

- 玩家属性系统（金币消耗）
- 背包系统（消耗升级材料）

---

## 10. 天气与事件系统

**职责：** 管理每日天气生成、随机事件触发。

### 数据结构

```
WeatherSystem
├── current_weather: WeatherType
└── daily_weather_generated: bool

EventSystem
├── pending_events: Event[]
└── event_cooldowns: Map<EventId, int>
```

### 天气配置

| ID | 天气 | 概率 | 效果 |
|----|------|------|------|
| weather_sunny | 晴天 | 40% | 正常 |
| weather_rain | 小雨 | 25% | 农田自动浇水，钓鱼 -10% |
| weather_cloudy | 阴天 | 20% | 采矿经验 +20% |
| weather_storm | 暴风雨 | 10% | 农作物减产 10%，不能钓鱼 |
| weather_typhoon | 台风 | 5% | 农作物减产 35%，不能外出 |

### 事件配置

| ID | 事件 | 触发概率 | 触发时间 | 效果 |
|----|------|---------|---------|------|
| event_storm | 暴风雨 | 8%/天 | 08:00 | 农作物减产 10% |
| event_typhoon | 台风 | 3%/天 | 08:00 | 农作物减产 35% |
| event_rain | 小雨 | 20%/天 | 08:00 | 不用浇水 |
| event_traveler | 旅行商人 | 周五 100% | 08:00 | 出售稀有种子/道具 |
| event_pest | 虫害 | 累计3天未浇水 | 08:00 | 作物减产 20% |
| event_chest | 挖到宝箱 | 采矿时随机 | — | 随机金币/矿石 |
| event_cavein | 矿洞塌方 | 采矿时 5% | — | 失去当日所有矿石 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| generateDailyWeather() | 每天 06:00 生成当日天气 |
| checkEventTrigger(gameTime, player) | 在 08:00 检查是否触发事件 |
| applyWeatherEffects(weather, farm) | 应用天气对农田的影响 |
| applyEventEffects(event, player) | 应用事件效果 |
| canFish(weather) | 暴风雨/台风时返回 false |
| canGoOutside(weather) | 台风时返回 false |

### 依赖

- 时间系统（读取当前时间，判断 08:00 触发）
- 种菜系统（应用天气/事件对作物的影响）

---

## 11. 任务系统

**职责：** 管理主线任务、每日任务、订单系统。

### 数据结构

```
QuestSystem
├── main_quests: Quest[]            // 主线任务进度
├── daily_quests: Quest[]           // 当日每日任务
└── orders: Order[]                 // 当前订单

Quest
├── id: string
├── name: string
├── description: string
├── type: QuestType                 // MAIN / DAILY / ORDER
├── objectives: Objective[]
├── rewards: Reward
└── is_completed: bool

Objective
├── type: string                    // "plant" / "fish" / "mine" / "sell" / "earn_gold"
├── target_id: string               // 物品 ID（可选）
├── target_count: int
└── current_count: int

Reward
├── gold: int
├── exp: { farming: int, fishing: int, mining: int }
└── items: ItemReward[]
```

### 主线任务列表

| 序号 | 任务 | 目标 | 奖励 |
|------|------|------|------|
| 1 | 继承农庄 | 完成新手引导 | 100 金 |
| 2 | 第一桶金 | 累计卖出 100 金币 | 50 金, 农业经验 +30 |
| 3 | 田园扩张 | 农田升级到 3 级 | 200 金 |
| 4 | 钓鱼高手 | 钓到彩虹鳟鱼 | 150 金, 钓鱼经验 +50 |
| 5 | 矿坑探秘 | 采矿等级达到 3 级 | 300 金, 采矿经验 +50 |
| 6 | 集市大亨 | 累计交易额 5000 金币 | 1000 金 |

### 每日任务模板（随机 3 个/天）

| 模板 | 目标 | 奖励 |
|------|------|------|
| plant_x | 种植 X 根作物 | 20 金, 农业经验 +20 |
| catch_x | 钓到 X 条鱼 | 30 金, 钓鱼经验 +20 |
| mine_x | 挖到 X 块矿石 | 30 金, 采矿经验 +20 |
| sell_x | 在集市卖出 X 件物品 | 25 金 |
| earn_x | 获得 X 金币收入 | 50 金 |

### 核心逻辑

| 功能 | 说明 |
|------|------|
| generateDailyQuests() | 每天 06:00 生成 3 个随机每日任务 |
| updateProgress(type, targetId, count) | 某类行为发生时更新任务进度 |
| completeQuest(questId) | 完成任务，发放奖励 |
| checkMainQuestUnlocks() | 检查主线任务解锁条件 |
| getOrderReward(order) | 计算订单奖励（稀有订单翻倍） |

### 依赖

- 玩家属性系统（发放奖励）
- 背包系统（发放物品奖励）
- 种菜/钓鱼/采矿系统（触发进度更新）

---

## 12. 持久化系统

**职责：** 管理游戏存档的序列化与反序列化。

### 存档结构（JSON）

```json
{
  "version": "1.2",
  "player": {
    "name": "小农夫",
    "gold": 156,
    "satiety": 70,
    "current_location": "HOME",
    "farming_exp": 150,
    "farming_level": 2,
    "fishing_exp": 30,
    "fishing_level": 1,
    "mining_exp": 10,
    "mining_level": 1
  },
  "farm": {
    "level": 2,
    "capacity": 8,
    "plots": [
      {"crop": "tomato", "stage": 2, "watered": true, "fertilized": false}
    ]
  },
  "tools": {
    "hoe": {"durability": 45, "level": 1},
    "rod": {"durability": 28, "level": 1},
    "pickaxe": {"durability": 25, "level": 1}
  },
  "inventory": {
    "items": [
      {"id": "carrot", "count": 5},
      {"id": "fish_bass", "count": 2},
      {"id": "ore_iron", "count": 8}
    ]
  },
  "time": {
    "day": 12,
    "hour": 8,
    "minute": 30
  },
  "market": {
    "shop_favor": {"shop_seed": 5},
    "price_fluctuations": {}
  },
  "unlocked_areas": ["HOME", "FARM", "COAST", "TOWN", "MINE_SHALLOW"],
  "stats": {
    "total_income": 1200,
    "total_fish_caught": 34,
    "total_crops_harvested": 45,
    "total_ores_mined": 12
  }
}
```

### 核心逻辑

| 功能 | 说明 |
|------|------|
| saveGame(filePath) | 将当前游戏状态序列化为 JSON 写入文件 |
| loadGame(filePath) | 从 JSON 文件反序列化恢复游戏状态 |
| hasSave(filePath) | 检查存档文件是否存在 |
| deleteSave(filePath) | 删除存档文件 |

### 存档触发时机

| 事件 | 是否自动存档 |
|------|------------|
| 睡觉（进入下一天） | 是 |
| 退出游戏 | 是 |
| 完成主线任务 | 是 |
| 随时手动存档 | 是 |

### 依赖

- 所有子系统（读取/写入各系统状态）

---

## 子系统依赖关系图

```
                    ┌──────────────┐
                    │  持久化系统   │
                    └──────┬───────┘
                           │ 读写所有系统状态
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
    ┌──────────┐    ┌──────────┐    ┌──────────┐
    │  时间系统  │    │ 地图系统  │    │ 玩家属性  │
    └────┬─────┘    └────┬─────┘    └────┬─────┘
         │               │               │
    ┌────┼───────────────┼───────────────┼────────┐
    │    │               │               │        │
    ▼    ▼               ▼               ▼        ▼
┌──────┐ ┌──────┐  ┌──────────┐  ┌──────┐ ┌──────────┐
│天气系统│ │事件系统│  │ 玩家状态机 │  │工具系统│ │ 背包系统  │
└──┬───┘ └──┬───┘  └────┬─────┘  └──┬───┘ └──────────┘
   │        │           │           │
   ▼        ▼           ▼           ▼
┌──────────────────────────────────────────────┐
│              三大核心玩法系统                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  种菜系统  │  │  钓鱼系统  │  │  采矿系统  │   │
│  └──────────┘  └──────────┘  └──────────┘   │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
              ┌──────────────┐
              │   集市系统    │
              └──────┬───────┘
                     │
                     ▼
              ┌──────────────┐
              │   任务系统    │
              └──────────────┘
```

---

*文档版本：v1.0*
*创建日期：2026-08-22*
