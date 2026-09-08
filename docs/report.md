# Carrot Valley 项目结构分析报告

> 分析时间：2026-09-08
> 对照文档：`docs/subsys.md`（12 子系统规划）

---

## 一、子系统对照总览

| # | 子系统 | 磁盘位置 | 状态 | 说明 |
|---|--------|----------|------|------|
| 1 | 时间系统 | `src/model/timeService/` | ✅ 完整 | `GameDateTime` 日历结构体 + `TimeService` 帧驱动推进 + 定时回调调度器 |
| 2 | 玩家属性 | `src/controller/Player/` | ✅ 完整 | `Player` 类：位置/状态/饱食度/经验/背包 |
| 3 | 玩家状态机 | `src/model/Playerstates/` | ✅ 存在 | 状态枚举 + 状态逻辑 |
| 4 | 地图系统 | `src/model/Map/` | ✅ 存在 | `Position`、`Farmland`、`Mine`、`Coast`、`Home`、`Town` |
| 5 | 种菜系统 | `src/controller/Crop/` + `Farm/` + `Fertilizer/` + `Farming/` | ✅ 已实现 | 5 种作物（Carrot/Tomato/Cabbage/Pumpkin/Lingzhi）、2 种肥料、Farm/FarmLand、FarmingController |
| 6 | 钓鱼系统 | `src/controller/Fish/` + `Fishing/` | ✅ 已实现 | 5 种鱼（GrassCarp/KingCrab/Perch/RainbowTrout/Crucian）、FishingController |
| 7 | 采矿系统 | `src/controller/Mining_controller/` + `src/Data/Ore/` | ✅ 完整 | 完整实现 + 8 个 GoogleTest 测试套件 |
| 8 | 集市系统 | `src/controller/Market/` | ✅ 已实现 | Market、Shop、ShopItem |
| 9 | 工具系统 | `src/controller/Tool/` | ✅ 已实现 | Tool/ToolController/ToolConfig（锄头/鱼竿/镐子 3 件） |
| 10 | 天气与事件 | `src/controller/Weather/` | ✅ 完整 | Weather（5 种天气）+ EventSystem（7 种事件）+ WeatherController |
| 11 | 解析系统 | `src/View/Cmdparser/` | ✅ 完整 | CLI11 解析器 + 4 个 GoogleTest 测试套件 |
| 12 | 持久化系统 | `src/Tool/`（PlayerSerializer）+ `src/controller/Game/` | ⚠️ 部分 | Game 类已定义，PlayerSerializer 已实现，但 `main.cpp` 为空桩 |

### 结论

`docs/subsys.md` 规划的 12 个子系统**全部存在于磁盘**，大部分已有实质实现。第一次分析（基于 `read` 工具的陈旧缓存数据）错误地报告种菜/钓鱼/集市缺失，经 `type` 命令验证后已纠正。

---

## 二、关键构建问题

### 问题 1：`src/CMakeLists.txt` 当前状态异常

工作区中 `src/CMakeLists.txt` 包含以下无效引用：

```cmake
add_subdirectory(Objects)            # ← src/Objects/ 目录不存在
add_subdirectory(Controller/Objects) # ← src/Controller/Objects/CMakeLists.txt 已删除
```

这两个 `add_subdirectory` 会导致 CMake configure 阶段失败。

### 问题 2：Jerry 模块引用不存在的 `Time.h`

以下三个头文件引用 `#include "Time.h"` 并使用 `const Time& time`（带 `time.hour`、`time.day` 字段）：

- `src/controller/Farming/include/FarmingController.h`
- `src/controller/Fishing/include/FishingController.h`
- `src/controller/Market/include/Market.h`

**但 `Time.h` 在整个仓库中不存在。** 该类型在 TimeService 重构（commit `8a95a5d`）中被重命名为 `mud::time::GameDateTime`（位于 `game_time.h`），Jerry 模块未同步更新。

根 CMakeLists.txt 的 `include_directories` 中引用了 `src/model/Time/include`，该目录同样不存在。

### 问题 3：Object 类重复编译

两个 CMake 目标都编译 `Object.cpp`：

| 目标名 | 定义位置 | 源文件 |
|--------|----------|--------|
| `objects` | 根 `CMakeLists.txt` | `src/model/Objects/src/Object.cpp` |
| `model_objects` | `src/model/Objects/CMakeLists.txt` | 同上 |

如果两个目标同时链接到同一可执行文件，会产生重复符号定义。

### 问题 4：MSVC 专用编译选项

根 CMakeLists.txt 中有：
```cmake
add_compile_options(/utf-8)
```
此选项仅 MSVC 支持，GCC/Clang 会报错。应改为条件编译：
```cmake
if(MSVC)
    add_compile_options(/utf-8)
endif()
```

---

## 三、垃圾 / 过时文件

| 文件 | 问题 | 建议 |
|------|------|------|
| `src/controller/Objects/CMakeLists.txt` | 已从磁盘删除，但目录仍存在 | 删除整个 `src/controller/Objects/` 目录 |
| `src/controller/Objects/` | 空目录（CMakeLists 已删，无其他内容） | 删除 |
| `test/test_player.cpp` | 遗留旧测试（不在 `tests/` GoogleTest 套件中），使用错误 include 路径 `../src/tool/`（应为 `Tool/`） | 删除或迁移至 `tests/` 并修正路径 |
| `src/controller/Weather/include/weather.h`（旧版） | 旧天气头文件（空桩），与新版 `weather.h`（含 `mud::weather` 命名空间）共存 | 验证后删除旧版 |
| `src/controller/Weather/src/weather.cpp`（旧版） | 旧天气实现（空桩），与新版共存 | 验证后删除旧版 |
| `src/controller/Weather/src/weather_controller.cpp`（旧版） | 旧天气控制器（依赖已废弃接口） | 验证后删除旧版 |

---

## 四、代码质量问题

### 4.1 `main.cpp` 为空桩

```cpp
/**
 * @file main.cpp
 * @brief MUD 游戏命令行入口（最终入口文件，暂未编写）。
 */
```

整个可执行入口未实现，项目当前只能构建为库集合。

### 4.2 路径大小写不一致

- Git 跟踪路径使用大写：`Controller/`、`Model/`
- 磁盘实际目录为小写：`controller/`、`model/`
- Windows 不区分大小写，正常工作；Linux/macOS 会构建失败

### 4.3 `Game.h` 使用硬编码相对路径

```cpp
#include "../../../Model/Timeservice/include/game_time.h"
```

应改为 CMake target 提供的 include 路径（如 `#include "game_time.h"`）。

### 4.4 WeatherController 依赖链

`weather_controller` CMakeLists.txt 链接 `farm`（PUBLIC），`farm` 为 INTERFACE 目标链接 `crop`，`crop` 链接 `model_objects`。依赖链正确，但 `Farm` 的 `autoWater()` 方法仅在 WeatherController 中被调用，实际农田逻辑是否完整需验证。

---

## 五、推荐修复顺序

1. **修复 `src/CMakeLists.txt`** — 移除 `add_subdirectory(Objects)` 和 `add_subdirectory(Controller/Objects)`
2. **修复 `Time.h` 引用** — 在 `FarmingController.h`、`FishingController.h`、`Market.h` 中将 `#include "Time.h"` 替换为 `#include "game_time.h"`，`const Time&` 替换为 `const mud::time::GameDateTime&`
3. **消除 Object 重复编译** — 统一使用 `model_objects`（或 `objects`），移除另一个
4. **清理空目录** — 删除 `src/controller/Objects/`
5. **清理遗留测试** — 删除 `test/test_player.cpp`
6. **修复 `/utf-8`** — 改为 `if(MSVC)` 条件编译
7. **修正 `Game.h` 的硬编码 include 路径**

---

## 六、附录：磁盘实际目录结构

```
src/
├── controller/
│   ├── Crop/          ✅ Crop + 5 子类（Carrot/Tomato/Cabbage/Pumpkin/Lingzhi）
│   ├── Farm/          ✅ Farm + FarmLand
│   ├── Farming/       ✅ FarmingController
│   ├── Fertilizer/    ✅ Fertilizer + NormalFertilizer + AdvancedFertilizer
│   ├── Fish/          ✅ Fish + 5 子类（GrassCarp/KingCrab/Perch/RainbowTrout/Crucian）
│   ├── Fishing/       ✅ FishingController
│   ├── Food/          ✅ Food（Crop/Fish 的中间基类）
│   ├── Game/          ✅ Game 类（含时间持久化接口）
│   ├── Market/        ✅ Market + Shop + ShopItem
│   ├── Mining_controller/ ✅ 完整采矿系统 + 8 个测试套件
│   ├── Objects/       ⚠️ 空目录（CMakeLists 已删）
│   ├── Player/        ✅ Player + Move
│   ├── Tool/          ✅ Tool + ToolController + ToolConfig
│   └── Weather/       ✅ Weather + WeatherController + EventSystem
├── model/
│   ├── Bag/           ✅ Bag（库存）
│   ├── Map/           ✅ Position + 5 地图区域
│   ├── Objects/       ✅ Object 基类（model_objects 目标）
│   ├── Playerstates/  ✅ PlayerState
│   └── timeService/   ✅ GameDateTime + TimeService
├── View/
│   ├── Cmdparser/     ✅ CLI11 命令解析器 + 4 个测试套件
│   └── Statusview/    ✅ 状态视图
├── Tool/              ✅ PlayerSerializer
└── Data/
    └── Ore/           ✅ 矿石 JSON 数据文件

test/
└── test_player.cpp    ⚠️ 遗留旧测试（不在 tests/ 中）

tests/                 ✅ GoogleTest 套件（cmdparser × 4, timeservice × 8, mining_controller × 8）
```
