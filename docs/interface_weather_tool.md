# 事件 / 工具系统接口文档

> 状态：本文档对齐 `zhangmx` 分支当前已落地代码（`src/Controller/Weather`、`src/Controller/Tool`）。
> 天气枚举与逐日预报在本分支仍为留白（`weather.h` / `weather.cpp` 均为空文件），不在本文档的接口范围内，详见文末"现状缺口"。

## 1. 概述

工程采用 MVC 三层，本目录代码分为 Model（纯数据、纯逻辑）与 Controller（流程调度），依赖方向单向：`Controller -> Model / View`，Model 不回调上层。

当前构建内实际落地的两处能力：

- 事件模型 `mud::event`：采矿随机事件判定（宝箱翻倍、塌方中断）。
- 工具模型与控制器 `mud::tool`：锄头、鱼竿、矿镐三件工具的等级、耐久与使用。

采矿控制器把这两处作为可选依赖注入，驱动每次产出的收益结算，见第 6 节。

## 2. 目录结构

```text
src/Controller/Weather/
├── include/
│   ├── weather.h            // 空文件，天气预报留白
│   ├── weather_Controller.h // WeatherController 声明（未入构建）
│   └── event.h              // mud::event::EventSystem
└── src/
    ├── weather.cpp          // 空文件
    ├── event.cpp            // EventSystem::roll_mining_event 实现
    └── weather_Controller.cpp // 未入构建

src/Controller/Tool/
├── include/
│   ├── tool.h               // mud::tool::Tool
│   ├── tools.h              // mud::tool::ToolId / ToolConfig / kToolConfigs
│   └── tool_Controller.h    // mud::tool::ToolController
└── src/
    ├── tool.cpp             // Tool 实现 + kToolConfigs 定义
    └── tool_Controller.cpp  // ToolController 实现
```

## 3. 事件模型（`mud::event`）

文件：`src/Controller/Weather/include/event.h`

```cpp
namespace mud::event
{
    struct EventSystem
    {
        void roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance) const;
    };
}
```

判定规则（`event.cpp`，`rand_chance` 约定落于闭区间 `[1,100]`）：

| 事件   | 区间        | 概率  | 效果                 |
| ------ | --------- | :--: | -------------------- |
| 塌方   | `< 5` (1-4) | 约 5% | 中断本 tick 结算，结束会话 |
| 宝箱   | `>= 93` (93-100) | 约 8% | 本次产出的数量翻倍        |
| 其他   | 5-92      | 约 87% | 无事件                |

塌方与宝箱区间互不重叠，一次判定最多命中其一。

## 4. 工具模型（`mud::tool`）

文件：`src/Controller/Tool/include/tool.h`、`tools.h`

### 枚举与配置

```cpp
enum class ToolId : int { Hoe = 0, Rod = 1, Pickaxe = 2 };

struct ToolConfig
{
    ToolId id;
    const char* name;
    int max_durability;
    int durability_per_use;
    int max_level;
    int upgrade_cost[2];
    const char* upgrade_material[2];
    int upgrade_material_count[2];
    const char* repair_ore;
    int repair_ore_count;
    int repair_base_gold;
};

extern const ToolConfig kToolConfigs[3]; // 下标即 ToolId 枚举序
```

配置表（`tool.cpp` 当前值）：

| 工具      | name | 满耐久 | 每用消耗 | 等级上限 | 升级费(L2/L3) | 升级材料         | 修复矿石 |
| ------- | ---- | :-: | :-: | :-: | :-: | ------------ | ---- |
| Hoe     | 锄头   | 50  | 1  | 5  | 10 / 30 | wood ×2 → stone ×3 | copper |
| Rod     | 鱼竿   | 40  | 1  | 5  | 10 / 30 | wood ×2 → stone ×3 | copper |
| Pickaxe | 矿镐   | 20  | 1  | 5  | 10 / 30 | wood ×2 → stone ×3 | iron  |

升级费、升级材料、修复矿石字段为未来金币/背包系统接入预留，当前生成流程不使用。

### 类接口（单件工具）

```cpp
class Tool {
public:
    explicit Tool(ToolId id);
    bool use();                 // 扣一次耐久；损坏返回 false
    bool is_broken() const;     // 耐久不足以完成一次使用
    int level() const;  int max_level() const;
    int durability() const;  int max_durability() const;
    int durability_per_use() const;
    std::string name() const;
    int level_bonus() const;    // 加成 = 等级 - 1
    bool upgrade();             // 未达上限则 +1；满级返回 false
    void repair_fully();        // 耐久回满
};
```

`is_broken` 判据为 `durability < durability_per_use`，即耐久不足以支撑再一次使用即视为损坏。

## 5. 工具控制器（`mud::tool::ToolController`）

文件：`src/Controller/Tool/include/tool_Controller.h`

构造默认装配 3 件工具（下标对应 `ToolId`）。公开接口聚焦使用与查询：

```cpp
bool use_tool(ToolId id);       // 使用指定工具一次；损坏返回 false
bool is_broken(ToolId id) const;
int durability(ToolId id) const;
int level_bonus(ToolId id) const;
int level(ToolId id) const;
std::string name(ToolId id) const;
```

升级/修复留在 `Tool` 模型层（`upgrade` / `repair_fully`），控制器当前未透出；金币/背包校验接口尚未接入，属现状缺口。

## 6. 采矿集成

采矿控制器（`MiningController::produce`，`src/Controller/Mining_controller`）在每 tick 的每次产出依次执行：

1. 事件判定：若注入了 `mud::event::EventSystem`，调用 `roll_mining_event(chest, cave, 1..100)`；塌方则清空本 tick 全部产出并中断会话，宝箱则本次产出 `quantity` 翻倍。
2. 矿镐耐久：若注入了 `mud::tool::ToolController`，每次产出经 `use_tool(Pickaxe)` 扣 1 耐久；损坏则清空产出、结束会话。
3. 经验结算：`经验 = 矿石经验 * (1 + ToolController::level_bonus(Pickaxe))`，等级 1 加成 0。

`MiningController` 构造签名（`events`、`tools` 均允许 `nullptr`，此时跳过对应环节）：

```cpp
MiningController(
    const Ore::OreData&, const TimeService&,
    const mud::event::EventSystem* events = nullptr,
    mud::tool::ToolController* tools = nullptr);
```

集成行为由测试 `tests/mining_controller/event_tool/event_tool_test.cpp` 覆盖：宝箱/塌方边界、工具耐久与等级加成、矿镐损坏中断会话、事件注入不影响产出形态。

## 7. 命名空间与工程约定

- 统一 `mud::event` / `mud::tool`，方法 snake_case，与全项目新的 `mud::` 模块风格一致。
- 源文件含 UTF-8 中文，MSVC 编译需带 `/utf-8`（各模块 CMakeLists 已设置）。
- 依赖注入一律经构造传入，跨模块不直接 include 对方具体头文件。

## 8. 现状缺口

本文档描述之外，当前分支还有以下留白，未纳入任何构建目标：

- 天气枚举与逐日预报（`weather.h`、`weather.cpp` 为空），以及 `WeatherController`（`weather_Controller.h/cpp`）。`weather_Controller.cpp` 中的 `farm_.autoWater()`、`time_.day()` / `time_.hour()` 与现有 `Farm` / `TimeService` 接口不一致，接入前需对齐（种菜子系统成型后由真实 `Farm` 提供自动浇水）。
- 工具升级/修复链路：`ToolController` 未暴露 `upgrade` / `repair_fully`，也未接入金币与背包校验。