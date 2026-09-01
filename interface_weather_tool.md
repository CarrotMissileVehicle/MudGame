# 天气 / 随机事件 / 工具系统 —— 接口文档

> 状态：基础功能已完成，可独立编译运行（含演示 main）。
> 依赖的 `Player / Inventory / Farm` 目前为占位桩，接口为**协作约定**，待对应模块实现后替换。

---

## 1. 概述

本工程采用 **MVC 三层**架构。天气与随机事件、工具两个系统分别拆为 **Model（纯数据+纯逻辑）** 与 **Controller（流程调度）**，层间依赖方向唯一：

```text
Controller ──► Model / View     （允许）
Model      ──► 上层            （禁止，Model 被动）
```

- **天气系统**：5 种天气 + 静态概率配置；驱动农田浇水、钓鱼、采矿、外出的加成/限制。
- **随机事件系统**：每日 08:00 固定事件 + 采矿中随机事件（宝箱/塌方）。
- **工具系统**：锄头/鱼竿/矿镐 三件的使用、升级、修复。

---

## 2. 目录结构

```text
src/controller/weather/
├── include/
│   ├── weather.h              // 天气模型 WeatherType/WeatherConfig/Weather
│   ├── event.h                // 事件模型 EventType/EventScope/EventSystem
│   └── weather_controller.h   // WeatherController
└── src/
    ├── weather.cpp
    ├── event.cpp
    └── weather_controller.cpp

src/controller/tool/
├── include/
│   ├── tool.h                 // 工具模型 ToolId/ToolConfig/Tool
│   ├── tools.h                // 静态配置表 kToolConfigs / kToolCount
│   └── tool_controller.h      // ToolController
└── src/
    ├── tool.cpp
    └── tool_controller.cpp
```

---

## 3. 天气 Model（`mud::weather`）

文件：`src/controller/weather/include/weather.h`

### 枚举与配置表

```cpp
enum class WeatherType { Sunny, Rain, Cloudy, Storm, Typhoon }; // 晴/小雨/阴/暴风雨/台风

struct WeatherConfig {
    WeatherType type;
    const char* name;
    double probability;          // 当日出现概率
    bool   auto_water;           // 是否自动给农田浇水
    double crop_loss;            // 农作物减产比例
    double mining_exp_bonus;     // 采矿经验加成
    double fishing_penalty;      // 钓鱼成功率减益
    bool   can_fish;             // 能否钓鱼
    bool   can_go_outside;       // 能否外出
};
```

### 类接口

```cpp
class Weather {
public:
    void generate_daily();                  // 按概率生成当天天气（跨天 06:00 调用）
    WeatherType current() const noexcept;
    std::string current_name() const;
    bool   can_fish() const noexcept;       // 晴/小雨/阴可钓；暴风雨、台风不可
    bool   can_go_outside() const noexcept; // 仅台风不可外出
    bool   auto_water() const noexcept;
    double crop_loss_rate() const noexcept;
    double mining_exp_bonus() const noexcept;
    double fishing_penalty() const noexcept;
};
```

### 概率配置（`weather.cpp`）

| 天气 | name | 概率 | 自动浇水 | 减产 | 采加成 | 钓减 | 可钓鱼 | 可外出 |
|------|------|------|:---:|:---:|:---:|:---:|:---:|:---:|
| Sunny  | 晴天   | 40% | –   | –    | –    | –    | ✅ | ✅ |
| Rain   | 小雨   | 25% | ✅  | –    | –    | 10%  | ✅ | ✅ |
| Cloudy | 阴天   | 20% | –   | –    | 20%  | –    | ✅ | ✅ |
| Storm  | 暴风雨 | 10% | –   | 10%  | –    | –    | ❌ | ✅ |
| Typhoon| 台风   | 5%  | –   | 35%  | –    | –    | ❌ | ❌ |

---

## 4. 随机事件 Model（`mud::event`）

文件：`src/controller/weather/include/event.h`

### 枚举与配置表

```cpp
enum class EventType { Storm, Typhoon, Rain, Traveler, Pest, Chest, CaveIn };

enum class EventScope { Daily08, Mining };   // 每日 08:00 / 采矿中

struct EventConfig {
    EventType type;
    EventScope scope;
    const char* name;
};
```

### 类接口

```cpp
class EventSystem {
public:
    void generate_daily_events(int day_of_week, bool neglect_water, int rand_chance);
    bool has_event(EventType type) const noexcept;
    bool is_traveler_active() const noexcept;
    void roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance);
};
```

### 判定规则（`event.cpp`）

| 事件 | name | 触发条件 |
|------|------|----------|
| Storm   | 暴风雨 | 每日判定，概率 8% |
| Typhoon | 台风   | 每日判定，概率 3% |
| Rain    | 小雨   | 每日判定，概率 20% |
| Traveler| 旅行商人 | 周五必然触发 |
| Pest    | 虫害   | 累计 3 天未浇水（`neglect_water=true`）时才判定 |
| Chest / CaveIn | 宝箱/塌方 | 采矿中，`rand_chance` 门槛判定 |

> `rand_chance` 约定取值 `1..100`。宝箱概率数值目前实现与 `>=80` 有关，见文末"待确认"。

---

## 5. 天气与事件 Controller（`WeatherController`）

文件：`src/controller/weather/include/weather_controller.h`

```cpp
WeatherController(mud::Farm& farm);       // 构造注入农田引用

void update(int day, int hour);           // 每帧调用；
                                          //   06:00 生成当天天气并自动浇水
                                          //   08:00 触发当天每日事件

// 天气查询（转发至天气 Model）
mud::weather::WeatherType weather() const;
std::string weather_name() const;
bool  can_fish() const;
bool  can_go_outside() const;
bool  auto_water() const;
double crop_loss_rate() const;
double mining_exp_bonus() const;
double fishing_penalty() const;

// 事件查询
bool  has_event(mud::event::EventType type) const;
bool  is_traveler_active() const;
std::vector<std::string> today_event_names() const;

// 采矿随机事件（宝箱/塌方）
void roll_mining_event(bool& found_chest, bool& cave_in);
```

内部状态缓存 `last_weather_day_` / `last_event_day_`，保证跨天只生成一次、08:00 只触发一次（由调用方传入 `day/hour` 驱动）。

---

## 6. 工具 Model（`mud::tool`）

文件：`src/controller/tool/include/tool.h`、`tools.h`

### 枚举与结构

```cpp
enum class ToolId { Hoe = 0, Rod = 1, Pickaxe = 2 };   // 锄头/鱼竿/矿镐

struct ToolConfig {
    ToolId id;
    const char* name;
    int max_durability;         // 初始/最大耐久
    int durability_per_use;     // 每次使用消耗
    int max_level;
    int upgrade_cost[2];
    const char* upgrade_material[2];
    int upgrade_material_count[2];
    const char* repair_ore;
    int  repair_ore_count;
    int  repair_base_gold;
};
```

### 静态配置表（`kToolConfigs`）

| 工具 | name | 耐久 | 每用消耗 | 最高级 | 升级费(L2/L3) | 升级材料 | 矿修 | 修复基准 |
|------|------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Hoe     | 锄头 | 50 | 1 | 3 | 50 / 100     | 铁矿×5 → 银矿×3 | 铁矿×2 | 50 |
| Rod     | 鱼竿 | 30 | 1 | 3 | 80 / 200     | 铁矿×5 → 银矿×3 | 银矿×2 | 80 |
| Pickaxe | 矿镐 | 25 | 2 | 3 | 80 / 200     | 铁矿×5 → 银矿×3 | 铁矿×2 | 80 |

### 类接口

```cpp
class Tool {
public:
    explicit Tool(ToolId id);
    bool use();                  // 扣一次耐久；损坏返回 false
    bool is_broken() const;      // 判定耐久耗尽
    int  level() const;  int max_level() const;
    int  durability() const;  int max_durability() const;
    int  durability_per_use() const;
    const char* name() const;
    int  level_bonus() const;    // 加成 = 等级 - 1
    bool upgrade();              // 升一级（调用方先校验钱/材料）
    void repair_fully();         // 耐久回满
};
```

---

## 7. 工具 Controller（`ToolController`）

文件：`src/controller/tool/include/tool_controller.h`

```cpp
ToolController(mud::Player& player, mud::Inventory& inventory);  // 注入金币/背包

// ---- 持有：每类工具可同时持有多件（工具继承物品基类 Object）----
std::size_t tool_count(mud::tool::ToolId id) const; // 该类型当前持有件数
bool add_tool(mud::tool::ToolId id);                // 新增一件同类型工具

// ---- 使用：自动选一件未损坏的，一件用尽时自动换下一件 ----
bool use_tool(mud::tool::ToolId id);                   // 自动选第一件未损坏的；全部损坏则 false
bool use_tool(mud::tool::ToolId id, std::size_t slot); // 明确使用第 slot 件
bool is_broken(mud::tool::ToolId id) const;            // 该类型是否已无可用工具（全部损坏）
std::size_t broken_count(mud::tool::ToolId id) const;  // 该类型已损坏件数

// ---- 查询（slot 默认主件 = 0）----
int  level(mud::tool::ToolId id, std::size_t slot = 0) const;
int  durability(mud::tool::ToolId id, std::size_t slot = 0) const;
int  max_durability(mud::tool::ToolId id, std::size_t slot = 0) const;
int  level_bonus(mud::tool::ToolId id, std::size_t slot = 0) const;
std::string name(mud::tool::ToolId id) const;

// ---- 升级 / 修复（slot 默认主件 = 0）----
bool upgrade(mud::tool::ToolId id, std::size_t slot = 0);         // 先扣金币→查验材料→够才升级；不足自动退钱
bool repair(mud::tool::ToolId id, bool use_ore, std::size_t slot = 0); // use_ore=true 用矿石(免金币)，否则按损耗比例收金币
```

内部按 `ToolId` 持有 `std::vector<Tool>`（默认每类 1 件），支持同时持有多件。
`Tool` 继承物品基类 `Object`（`src/model/Objects/include/Object.h`），故工具是可持有的物品；其耐久以 `Tool::durability` 为准。

---

## 8. 依赖桩契约（协作接口）

下列模块未实现，当前为 **header-only 占位桩**（队友实现后接口不变即可替换）：

| 头文件 | 契约接口 | 说明 |
|--------|----------|------|
| `src/models/player/include/player.h` | `spend_gold(int)` / `add_gold(int)` / `gold()` | 工具升级/修复扣金币 |
| `src/models/inventory/include/inventory.h` | `has_item(id,count)` / `remove_item` / `add_item` | 升级材料校验与消耗 |
| `src/models/farm/include/farm.h` | `auto_water()` | 雨天自动浇水 |

---

## 9. 使用示例

### 主循环接入（`GameController`）

```cpp
mud::Farm farm;
WeatherController weather(farm);
// 每帧：
weather.update(day, hour);

// 种菜：产量用 weather.crop_loss_rate()，浇水看 weather.auto_water()
// 钓鱼：if (weather.can_fish()) 钓，结算套 weather.fishing_penalty()
// 采矿：经验用 weather.mining_exp_bonus()；每次产出调 weather.roll_mining_event(c, k)
```

### 集市铁匠铺

```cpp
mud::Player player;
mud::Inventory inventory;
ToolController tool(player, inventory);

// 多件持有：开局再添一把矿镐，共 2 件
tool.add_tool(ToolId::Pickaxe);
std::size_t n = tool.tool_count(ToolId::Pickaxe);      // 2

// 挖矿：自动选一件未损坏的；第一件耐久用尽自动换第二件，不必急着修
bool ok = tool.use_tool(ToolId::Pickaxe);
if (tool.is_broken(ToolId::Pickaxe))
    std::printf("本类型全部损坏：%zu 件，请修复或再添置\n", tool.broken_count(ToolId::Pickaxe));

bool up = tool.upgrade(ToolId::Hoe, /*slot=*/0);          // 升主件一级
bool r  = tool.repair(ToolId::Pickaxe, /*use_ore=*/true, /*slot=*/0); // 用矿石修第 1 件
```

---

## 10. 待确认 / 已知问题

1. **宝箱概率与注释不符**：`event.cpp` 中 `roll_mining_event` 判定 `rand_chance >= 80`（≈21%），但注释写"固定 8%"。需按 `proj.md` 确认宝箱概率数值（如需 8%，应改 `>= 92`）。
2. **中文编码**：含 UTF-8 中文的文件必须带 `/utf-8` 编译，否则 MSVC 按 GBK 读取会报错。
3. **命名空间**：本系统统一 `mud::weather` / `mud::event` / `mud::tool`，与全项目 `mud` 风格一致。