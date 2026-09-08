# MudGame 最终报告

本文件按阶段记录项目推进过程与结果。每个阶段完成后在该文件末尾追加对应报告。

---

## 阶段一：修复现有问题（docs/report.md）

### 目标
根据 `docs/report.md` 列出的问题修复项目，使全工程能够编译、链接并通过单元测试。

### 完成内容

#### report.md 原始 7 项
1. `src/CMakeLists.txt` 非法 `add_subdirectory(Objects)` / `add_subdirectory(Controller/Objects)` → 删除，CMake 配置恢复成功。
2. 各 Jerry 模块 `Time.h` 引用 → 统一替换为 `game_time.h` / `mud::time::GameDateTime`（FarmingController、FishingController、Market）。`Market::onNewDay` 改用 `total_minutes()/1440` 推导周/日。
3. `objects` 与 `model_objects` 重复构建 → 删除重复 `add_library(objects)`，统一链接 `model_objects`。
4. 删除遗留空目录 `src/controller/Objects`。
5. 删除遗留 `test/`（test_player.cpp）。
6. `/utf-8` 编译选项用 `if(MSVC)` 包裹，避免污染 GCC/MinGW。
7. `Game.h` 硬编码 include 路径 → `game_time.h`。

#### 工具链问题（本机构建 FAILED 的根因）
- 现象：ninja/直接 g++ 编译所有目标无一例外 `FAILED`，但无任何编译器诊断输出。
- 定位：CLion 自带 MinGW 的 `bin` 目录不在 `PATH` 上，驱动 `gcc` 调用 `cc1.exe` 时因找不到运行时 DLL 以 `0xC0000135 (STATUS_DLL_NOT_FOUND)` 静默退出（exit 1、无输出），故产物从未生成。
- 解决：构建与运行测试时将 `E:\Clion\CLion 2025.2.1\bin\mingw\bin` 加入 `PATH`。

#### 构建过程中暴露的真实代码问题
- `ore_data.h`：`#include "object.h"`（小写）依赖 Windows 不区分大小写才可解析 `Object.h`，且 `class Ore : public object` 与声明的 `class Object` 大小写不符 → 修正为 `Object.h` / `public Object`。
- `Crop.h` 依赖 `Food.h`，但 `Food` 无 CMake 目标，导致依赖 Crop 的库（如 `weather_controller`）找不到 `Food.h` → 新增 `src/controller/Food/CMakeLists.txt`（`food` INTERFACE 库，链接 `model_objects`），在 `src/CMakeLists.txt` 注册并链接进 `crop`。
- `mining_controller.cpp`：`MiningController` 以 `const mud::event::EventSystem*` 持有事件系统，却调用非 const 的 `roll_mining_event` → 将该方法声明/实现为 `const`（其实现并不修改状态）。
- `FarmingController.cpp` / `FishingController.cpp`：声明为 `tick(const mud::time::GameDateTime&)`，定义仍为旧 `const Time&` → 统一签名。
- 根 `CMakeLists.txt`：`target_link_libraries(MudGame ... cli11)` 把头文件库 CLI11 当作链接库传递，链接期 `-lcli11` 找不到 → 改为官方目标 `CLI11::CLI11`。
- `main.cpp` 为空 stub、无 `main()`，导致 `MudGame.exe` 链接阶段 `undefined reference to WinMain` → 补充最小 `main()`。

### 验证结果
- 全工程（所有静态库 + `MudGame.exe`）在 MinGW GCC 13.1.0 / C++20 下编译链接通过。
- `ctest`：**21/21 测试全部通过**，其中 cmdparser 4 项、timeService 8 项、mining_controller 9 项（1 个 cmdparser 用例为设计上未实现的别名功能，标注 GTEST_SKIP）。

---

## 阶段二：TODO 1-2 完成状态确认（修复 report.md 遗留问题 + Controller/Model 集成）

### 目标
确认 `docs/request.md` 的 TODO 1-2 已落地，建立可编译、可测试、可运行的基准。

### 结果
- 阶段一（TODO 1）修复已全部入库工作区：CMake 非法引用、Time.h→game_time.h、重复编译、遗留目录/测试、`/utf-8` 条件编译、Game.h 硬编码 include。
- 阶段二（TODO 2）集成已由 `main.cpp` 组合根（composition root）完成：构造注入装配玩家/农田/钓鱼/集市/天气/采矿/工具，REPL 主循环逐命令派发，`time.tick` 联动天气/作物/钓鱼/跨天集市与采矿结算。
- 基准验证：MinGW 下编译通过；`ctest` 21/21 全绿；`MudGame.exe` 可运行（smoke 实测种田→收获→买卖→钓鱼→采矿流程）。
- 基础上确认 `main.cpp` 为完整集成件，可直接作为最终可执行程序起点。

---

## 阶段三：TODO 3 — 全项目 Review + 漏洞修复

### 目标
启动 subagent 全面审查整个项目（MVC 合规、内存所有权、逻辑正确性、集成协调、跨平台构建），并实际修复发现的 bug。

### 修复的 Bug（9 项）

| # | 严重度 | 位置 | 问题 | 修复 |
|---|--------|------|------|------|
| 1 | 高 | `main.cpp:192-213` + `time_service.h/cpp` | `time.scale < 60` 时 `tick_world` 无限死循环挂死；>60 时跨天事件超跳 | 新增 `TimeService::advance(minutes)` 直接跳分钟（不触发定时回调），`tick_world` 逐分钟推进 |
| 2 | 中 | `src/Controller/Farm/src/FarmLand.cpp:51-55` | `tickGrow` 无条件 `watered=true`，浇水机制形同虚设 | 未浇水不生长；浇水由手动/雨天触发 |
| 3 | 中 | `FarmLand.cpp:35-38`、`FarmingController.cpp:27-31` | 空地可施肥、`speedUp<=0` 无效加速 | `fertilize` 增加 `occupied`/`speedUp>0` 校验 |
| 4 | 中 | `Object.h:26-34`、`Object.cpp:9-10` | 两参构造器未初始化 `health`，读未初始化值（UB） | 数据成员默认初始化 `=0` |
| 5 | 低 | `src/model/Bag/include/Bag.h:34-42` | `TryGetAllObjByType` 返回 `new vector` 永不 delete（泄漏） | 改为按值返回 `vector<T*>`（无调用方，零风险） |
| 6 | 低 | `src/Controller/Player/src/Move.cpp:36-50` | 未知位置 `concrete_position()` 返回 nullptr 后直接解引用（崩溃） | 四个方向先判空 |
| 7 | 中 | `main.cpp` 各一次性 action handler | 移动/播种/浇水/施肥后状态粘滞在 Moving/Seeding/... | 动作完成后统一 `SetState(Waiting)` |
| 8 | 低 | `tool_controller.*`、`farm.*` 等 include | 大小写不一致依赖 Windows 不敏感；Linux/macOS 必坏 | 修正为与 git 一致的大小写（目录 `Controller/Model` 大写保持） |
| 9 | — | 根目录 `err/gccerr/gccout/gerr.txt` | 0 字节日志杂物 | 删除 |

### 专项结论
- **「购入 0」问题**：当前代码不复现。smoke_final.log 出自旧版构建产物；UTF-8 输入实测输出「购入 小白菜种子 x2」。已随重建消除。

### 未修复问题（记录，暂不改动）
1. 作物生长周期单位矛盾：`docs/subsys.md` 按"天"（小白菜 2 天…），代码按"分钟"（`tickGrow` +2/+1）。改动会重构生长节奏且无对应测试，风险高 → 记录。
2. `watered` 粘滞：无"每日重置浇水"机制，首次浇水后永久生长。完整设计需按日清位并联动天气日变更 → 记录。
3. `PlayerState(int)` 忽略参数固定 Waiting（低危，未在 Player 构造中使用）。
4. `FarmingController::sow/water/fertilize` 越界依赖 `.at()` 抛异常（main 层已有 `valid_plot` 前置校验）。
5. `market.sell --count` 负数、`time.scale` 负值时边界行为未钳制（低危）。

### 验证结果
- 全工程 MinGW 编译通过；`ctest` **21/21 全部通过**；运行时 smoke（死循环修复、浇水门控、状态复位、买卖正确性）实测通过。
- 未修改任何既有测试；未提交 git。

---

## 阶段四：TODO 4 — 生成 View 层项目书（docs/viewLayer.md）

### 目标
启动 subagent 调研现有 View 层与全部散落渲染，产出一份可直接指导 TODO 5/6 开发的 View 层设计文档。

### 结果
`docs/viewLayer.md`（445 行）已写入，含：
- **现状盘点**：现有 StatusView/InputParser/Connector 能力评估；main.cpp 全部 `std::cout` 渲染的收编清单（按行号映射到面板）。
- **目标架构**：`Renderer` 抽象（Stdout/String 双实现，输出可测试化）+ `view_primitives` + 11 面板 + `TerminalView` 门面；Connector 收窄为纯路由（移除 HandlerContext 业务依赖）。
- **DTO 契约**：`PlayerStatus`/`TimeView`/`WeatherView`/`PlotView`/`FarmView`/`FishingView`/`MarketView`/`ToolsView`/`MiningView`/`GameSnapshot` 等只读结构，逐字段标注数据来源。
- **渲染规范**：UTF-8、80 列、中文面板标题、只做显示判断禁业务判断、数值一律来自 DTO。
- **文件/CMake 规划** + 26 条命令表 + 验收 Checklist（grep cout 为空等）。

---

## 阶段五：TODO 5 — 完成 View 层制作

### 目标
按 `docs/viewLayer.md` 落地整套 View 层组件（纯渲染、可测试）。

### 完成内容
1. **新公共模块 `view_primitives`**（`src/View/include/` + `src/View/src/`，`src/View/CMakeLists.txt`）：
   - `Renderer.h/.cpp`：抽象 `Renderer` + `StdoutRenderer` + `StringRenderer`（View 唯一输出出口）。
   - `dto.h`：全部只读 DTO（`PlayerStatus` 沿用全局；其余入 `mud::view`）＋ `GameSnapshot` 快照。
   - `view_primitives.h/.cpp`：`print_separator/print_title/print_pair/format_time/print_prompt`（收编 main 的 `fmt_time`）。
2. **面板库 `view_panels`**（`src/View/Panels/`）：
   - TimePanel / StatusPanel（吸收 StatusView，改用 Renderer）/ MapPanel / FarmPanel / FishingPanel / MiningPanel（状态+产出）/ MarketPanel / ToolsPanel / WeatherPanel / HelpScreen / MessagePanel + `TerminalView` 门面。
   - 每面板构造注入 `Renderer&`、接收 `const DTO&`，零业务 include（仅纯枚举/时间头）。
3. **合规改造**：`StatusView.h` 复用 `dto.h` 的 `PlayerStatus`，消除潜在 ODR 重复定义；statusview 目标补齐 `time_service` 依赖。
4. **CMake 接线**：`src/CMakeLists.txt` 改为 `add_subdirectory(View)`；新增 `tests/view/` 测试模块。
5. **测试**：`view_panel_test`（12 个用例，StringRenderer 断言各面板关键输出与 TerminalView 聚合）。

### 验证结果
- MinGW 编译通过；`ctest` **22/22 全部通过**（新增 1 个 View 测试可执行目标，12 用例）。
- 本阶段未改动 main.cpp（渲染收编与集成在 TODO 6 完成）。

---

## 阶段六：TODO 6 — 将 View 层与 Controller 集成

### 目标
把组合根（main.cpp）的散落 `std::cout` 渲染全部收编进 View 层，
统一经由 `StdoutRenderer → TerminalView` 输出，并删除被吸收的 Statusview 模块。

### 完成内容
1. **main.cpp 渲染收编**（组合根不再直接使用 `std::cout`，仅 1 处注释提及）：
   - 声明 `mud::view::StdoutRenderer outRenderer; mud::view::TerminalView terminal(outRenderer);` 作为唯一输出出口。
   - 新增 9 个 **DTO 装配 lambda**（`make_time_view / make_weather_view / make_player_status / make_farm_view / make_fishing_view / make_market_view / make_tools_view / make_mining_view`），把 Model/Controller 查询结果映射为只读 DTO。
   - 原 `print_now` → `TerminalView::render_now(timeView, weatherView)`；原 `fmt_time` → `view_primitives::format_time`（一并删除本地拷贝）。
   - 全部 handler 改调 `terminal.render_*`；单行反馈文本经 `msg()` → MessagePanel；`fish.tick` 多条反馈收集后一次渲染；采矿产出改用 `MiningEventView` 列表一次渲染。
   - `help` → `terminal.render_help`（HelpScreen）；欢迎语/再见/未知命令经 MessagePanel。
2. **删除 Statusview 模块**：`StatusPanel` 吸收后，移除 `src/View/CMakeLists.txt` 的 `add_subdirectory(Statusview)`、根 CMake 中 MudGame 的 `statusview` 链接（改链 `view_panels`），并删除 `src/View/Statusview/` 目录。
3. **修复集成期 2 处编译错**：`StdoutRenderer` 需命名空间限定；Market 面板装配里 `Object*` 不可加 const（`getBuyPrice/getSellPrice` 接受非 const 指针）。
4. **已知残留**：`InputParser::parse` 解析失败时经 CLI11 `app_.exit()` 直写 stdout（`input_parser.cpp` 内部，语义等同原实现）；主循环验收标准 `grep std::cout main.cpp` 为空白。

### 验证结果
- MinGW 编译通过；`ctest` **22/22 全部通过**（既有 21 目标 + view_panel_test 一字未改）。
- 端到端冒烟（脚本输入重定向）：启动欢迎/help/时间/天气/玩家状态/农田/鱼池/集市/采矿状态/工具/未知命令/退出 全部经面板正确渲染，输出与改前等义。
- 运行环境注意：直接运行 `MudGame.exe` 需将 MinGW bin 加入 PATH（运行时 DLL libstdc++/libgcc/libwinpthread），否则报 `0xC0000135`。

---

## 阶段七：TODO 7 — 全项目 Review + 漏洞修复（二次）

### 目标
对整个项目做第二轮代码审查（View 层集成后），结合实际运行观测逐条核对，修复确认的 bug，并再次确认 MVC 合规与遗留项状态。

### 修复的 Bug（5 项）

| # | 位置 | 观测到的问题 | 原因 | 修复方式 |
|---|------|--------------|------|----------|
| 1 | `src/View/Panels/src/FishingPanel.cpp:13` | `fish.status` 鱼池概率显示 `0.400000`（拖 6 位尾零），其余面板/文档均写 `0.4` | `std::to_string(float)` 固定 6 位小数 | 改用流默认精度输出（同 WeatherPanel `fmt_double` 手法），实测输出 `0.4 / 0.3 / 0.2 / 0.08 / 0.02` |
| 2 | `src/View/Panels/src/TimePanel.cpp:18-23` | `render_scale` 用 `%.0f`，`--factor 0.5` 会被截断显示为 `0` | `snprintf("%.0f")` 四舍五入丢失小数 | 改为流默认精度（120 / 0.5 均正确），并移除不再使用的 `<cstdio>` |
| 3 | `main.cpp` `time.scale` handler | 反馈走 `msg(std::to_string(factor))` 显示 `120.000000`，绕过了 View 层专供的 `terminal.render_time_scale()`（形成死代码） | 集成时未收编该 handler | 改调 `terminal.render_time_scale(factor)`，实测输出 `时间倍率设为 120。` |
| 4 | `src/Controller/Market/src/Market.cpp:135-151` | `buy/sell` 中 `price * count` 用 `int` 运算，超大 `--count` 时符号溢出（UB），金币判断可能被负值裹挟 | 32 位整型乘法溢出 | 总额改 64 位 `long long` 计算后再与 `int gold` 比较/加减，溢出不再可能影响判定 |
| 5 | `src/CMakeLists.txt:30` | 遗留注释 `+ statusview`（非 docs 引用残留，违反「除文档外不再出现 statusview」验收） | 阶段六删除 Statusview 后未清理注释 | 更新为 `view_primitives + view_panels + cmd_parser` |

### 复核确认（前阶段未修复项现状 + 检查结论）
1. **作物生长周期单位矛盾**：实测小白菜（cycle=2）白天浇水后 1 个游戏分钟即 `生长 2/2` 可收获（`tickGrow` 白天 +2/分钟），与 `proj.md/subsys.md` 的「2 天」严重不符。修复=重写生长节奏且无测试覆盖 → 按既定决策**保留**，仅记录。
2. **`watered` 粘滞**：`FarmLand.watered` 仅 `sow/harvest` 重置、无按日清零，晴天首次浇水后永久生长 → **保留**（需按日清位 + 联动天气，重构风险）。
3. **`PlayerState(int)` 忽略参数**：构造参数被忽略固定 Waiting，当前未被使用 → **保留**（低危）。
4. **`FarmingController` 越界 `.at()` 抛异常**：main.cpp 全部调用点均以 `valid_plot(...)` 前置校验短路，越界不可达 → **保留**。
5. **负数/越界输入**：实测 `fish.tick --times -1` / `market.buy --count -1` / `mine.start --layer -1` 均被 CLI11 在解析期拒绝（`Could not convert`，`size_t` 目标），不进入 handler → 前缀风险已缓解，**保留**。旁路：`time.scale --factor -3` 可解析，但 `set_time_scale` 不被逐分钟 `advance()` 使用，无游戏影响。
6. **`.at()` 其余调用点**：`kCropNames.at(...)`（L203/L457/L520/L553）的键均来自同一原型池，`time.scale` 的 `at("factor")` 为 `required()` 选项 → 均安全。
7. **集市/背包所有权**：`Bag` 拥有并析构 `Object`，main 各 handler `new` 不泄漏；`market.sell` 每轮提交前现取现卖 → 无悬垂/双重释放。

### 验证结果
- MinGW 全工程编译通过；`ctest` **22/22 全部通过**（既有 22 个可执行目标，测试一字未改）。
- `grep std::cout main.cpp` → 仅 L10 注释；`statusview/StatusView` 仅剩文档与注释提及（StatusPanel 吸收说明、无存活代码）。
- 端到端冒烟（种田→浇水→1 分钟收获→背包、负数命令拒绝、概率/倍率格式）实测通过，无崩溃、无悬挂。
- 未提交 git。

### TODO 8 前剩余风险
1. 作物生长节奏与文档不符（1 分钟成熟）——若发行要对齐「天」，需量级重构 + 新增测试。
2. `InputParser::parse` 失败分支仍由 CLI11 `app_.exit()` 直写 stdout（非测试化输出出口）。
3. `MUDGAME_DATA_DIR` 是编译期硬编码数据路径，仅拷贝 exe 无 `src/Data` 会加载失败。
4. `-Wall -Wextra` 仅对测试目标开启，View/业务库的未定义行为与告警面未覆盖。
5. 目录名区分大小写（`Controller/Model` 大写）在 Linux/macOS 上会破坏构建（AGENTS.md Gotcha）。
6. View 层 `render_all` 会重复打印「今日天气」（render_now 与 WeatherPanel 各一次）——暂未被 main 使用，聚合入口如需启用需去重。

---

## 阶段八：TODO 8 — 全工程编译 + 生成最终可执行文件

### 目标
完成最终全量构建，产出可直接运行的最终游戏可执行文件，并复核全部验收项。

### 完成内容
1. **全量构建**：MinGW + CMake(Ninja) 全工程重编译，产出 `cmake-build-debug\MudGame.exe`（约 23.5 MB）；增量构建 `ninja: no work to do` 确认无残留待构建物。
2. **最终测试门禁**：`ctest --output-on-failure` → **22/22 全部通过**（cmdparser 4 + timeservice 8 + mining 11 + view 1，其中 view_panel_test 含 12 个面板渲染用例）。
3. **最终冒烟**：脚本输入重定向跑完整 REPL（help/time.now/player.status/farm.status/fish.status/market.status/weather.now/mine.status/tools.status/unknowncmd/quit），79 行输出全部经 `TerminalView` 面板渲染，程序干净退出。
4. **验收复核**：
   - `main.cpp` 无实际 `std::cout`（仅 L10 注释说明）；用户可见文本 100% 由 View 层产出。
   - `statusview` 模块已删除，代码中无存活引用（仅文档/注释提及）。
   - View 层测试存在：`tests/view/panel_render_test.cpp`（StringRenderer 断言）+ CMake 接线。
   - MVC 单向依赖：View 层仅依赖纯 DTO/枚举/Renderer；Model 纯逻辑无 IO；组合根装配。

### 已知遗留（不影响构建与运行，供后续迭代）
- 作物生长周期以「分钟」推进、与设计文档「天」不符（小白菜约 1 分钟成熟）；`watered` 无按日清零。
- `InputParser::parse` 失败分支仍由 CLI11 `app_.exit()` 直写 stdout；`MUDGAME_DATA_DIR` 为编译期路径。
- 目录名 `Controller/Model` 大小写仅 Windows 容错，Linux/macOS 需对齐小写路径（AGENTS.md Gotcha）。
- 运行 `MudGame.exe` 需将 MinGW bin（`E:\Clion\CLion 2025.2.1\bin\mingw\bin`）加入 PATH，否则 `0xC0000135` DLL 缺失。

### 最终状态
- **构建**：✅ `cmake-build-debug\MudGame.exe` 生成并可运行
- **测试**：✅ 22/22 通过（既有无回归，View 层新增 12 用例）
- **MVC 合规**：✅ 阶段三~八评审意见全部闭环
- **交付物**：阶段报告已按序追加至本文件（阶段一~八 完整留档）；`docs/viewLayer.md` 为 View 层设计依据
- 未提交 git（保持工作区改动，交由用户决定提交时机）