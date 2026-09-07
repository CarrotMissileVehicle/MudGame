# 事件 / 工具系统 —— 现状与待办

> 分支：`zhangmx`
> 状态：本文档描述当前分支**已落地可编译运行**的事件与工具系统。上一版交接文档中针对 `src/models` 三个占位桩（`farm.h` / `inventory.h` / `player.h`）的"依赖桩替换契约"已随桩文件于 2026-09-05 删除而失效，本文档不再记录桩契约，改为对齐真实模块。
> 接口细节见根目录 `interface_weather_tool.md`。

## 0. 现状总览

| # | 事项 | 状态 |
| - | ---- | :--: |
| 1 | 采矿随机事件（宝箱 8% / 塌方 5%） | 已完成并接入采矿 |
| 2 | 工具模型与控制器（锄/竿/镐 使用与耐久） | 已完成并接入采矿（矿镐） |
| 3 | 天气枚举与逐日预报、WeatherController | 留白，未入构建 |
| 4 | 工具升级 / 修复接入金币与背包 | 留白 |

## 1. 已落地实现

### 1.1 事件模型（`mud::event`）

文件：`src/Controller/Weather/include/event.h`、`src/event.cpp`

`EventSystem::roll_mining_event(found_chest, cave_in, rand_chance)`：塌方 `rand_chance < 5`，宝箱 `rand_chance >= 93`，区间不重叠。在 `MiningController::produce` 中逐次产出判定：塌方清空本 tick 并中断会话，宝箱使当次产出数量翻倍。

### 1.2 工具模型与控制器（`mud::tool`）

文件：`src/Controller/Tool/include/{tool.h, tools.h, tool_Controller.h}`

`Tool` 单件工具持有等级与耐久，提供 `use` / `is_broken` / 查询 / `upgrade` / `repair_fully`。`ToolController` 默认装配三件（锄 50 耐久、鱼竿 40、矿镐 20，每用耗 1，等级上限 5），暴露使用与查询接口。采矿集成仅使用矿镐：每次产出 `use_tool(Pickaxe)` 扣 1 耐久，损坏即中断会话；经验按 `1 + level_bonus(Pickaxe)` 乘结算。

## 2. 留白与接入前提

### 2.1 天气枚举与逐日预报、WeatherController（未入构建）

`weather.h` / `weather.cpp` 为空文件；`weather_Controller.h/cpp` 未列入 `weather_controller` 库的构建源（该库仅编译 `event.cpp`）。`weather_Controller.cpp` 中引用的 `farm_.autoWater()`、`time_.day()` / `time_.hour()` 与当前 `Farm`（`src/Controller/Farm`，骨架态）和 `TimeService`（`mud::TimeService`，仅 `now().GameDateTime`）不一致。接入前提：

- 种菜子系统成型，由真实 `Farm` 提供 `autoWater()`；
- `TimeService` 补充按字段读取或由 `now()` 字段驱动判定；
- 将 `weather_Controller.cpp` 加入 `weather_controller` 库源即可编译联调。

### 2.2 工具升级 / 修复接入金币与背包（留白）

`Tool::upgrade` / `repair_fully` 已实现，但 `ToolController` 未透出，也未接入金币与背包校验。此前占位桩提供的 `spend_gold` / `has_item` / `remove_item` / `add_item` 接口已随桩删除，真实金币/背包入口以 `Player`（`src/Controller/Player`，含 `Bag`）为准，接入时以真实模块接口为准，不再沿用旧桩签名。

## 3. 工程约束

- 新模块统一 `mud::` 命名空间 + snake_case；继承 `Player / Bag / Farm` 等处改用对应模块既有风格（旧 Model 模块为全局 + PascalCase）。
- 源文件含 UTF-8 中文，MSVC 编译需带 `/utf-8`（各模块 CMakeLists 已设置）。
- 依赖方向保持 MVC 单向，跨模块依赖经构造函数注入。