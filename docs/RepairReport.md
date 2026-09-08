# 修复报告（RepairReport.md）

> 对应修复目标书 `docs/repair.md`，按阶段依次追加。每阶段完成时在本文件末尾追加该阶段报告。

---

## 阶段 1：修复 BUG（①天气/事件冲突 ②钓鱼不消耗体力）

### ① 天气与事件系统冲突

**排查结论**：事件系统 `mud::event` 的 `EventType` 与每日事件轮盘 `kDailyProb` 里存在与天气系统（`mud::weather::WeatherType`：晴天/小雨/阴天/暴风雨/台风）重叠的天气类条目（Storm 暴风雨、Typhoon 台风、Rain 小雨）。每日随机事件与天气各生成一次，导致“小雨”既出现在天气系统又被事件系统当成独立事件登记，形成冲突。

**修改**：
- `src/controller/Weather/include/event.h`
  - 从 `EventType` 中删除 `Storm` / `Typhoon` / `Rain`（天气类事件不再由事件系统表达）。
  - `kDailyEventConfigs` 只保留与天气无关的独立事件：旅行商人（周五）、虫害（连续 3 天未浇水）。
  - `generate_daily_events(int day_of_week, bool neglect_water)` 去掉 `rand_chance` 参数（不再需要天气类轮盘）。
  - 新增 `kDailyEventConfigCount`（=2）供测试断言配置约束。
- `src/controller/Weather/src/event.cpp`：删除加权轮盘 `kDailyProb`，仅保留周五/虫害的自洽触发逻辑；每天生成前清空前一日事件。
- `src/controller/Weather/src/weather_controller.cpp`：调用处改为 `event_.generate_daily_events(day_of_week(day_index), neglect_water)`；`roll_1_to_100()` 仍被 `roll_mining_event` 使用，予以保留。

**验证**：新增 `tests/weather/event_system_test.cpp` 断言任意普通日期不产生任何事件、周五仅旅行商人、未浇水仅虫害、次日清空，且事件系统配置中不出现天气类名称。`weather_event_test` 通过。

### ② 钓鱼不消耗体力

**修复**：`main.cpp` 新增常量 `kFishingSatietyCost = 5`。`fish.tick` 每轮垂钓在 `tickFish()` 前扣除 5 点体力（饱食度），并在进入前与每轮等待后双重检查 `satiety <= 0`——体力耗尽时提示“体力耗尽，钓鱼停止”并退出循环。

**验证**：初始体力 100 → 每轮 -5，理论最多连续 20 轮；代码审查 + `fish.tick` 前台循环逻辑走查确认。

---

## 阶段 2：添加背包堆叠逻辑（例如“小麦 x2”）

**方案**：在富对象基类上增加数量字段，背包 `AddObject` 对同名物品按堆叠合并。

**修改**：
- `src/model/Objects/include/Object.h` / `src/Object.cpp`：新增 `quantity`（默认 1）与 `GetQuantity()` / `SetQuantity()` / `AddQuantity()`。
- `src/model/Bag/include/Bag.h` / `src/Bag.cpp`：
  - `AddObject`：同名字段（名称相同）时向既有堆叠 `AddQuantity` 并 `delete` 传入对象；否则追加新堆叠。
  - 新增 `AddUnique`（读档还原专用，**不**合并）、`CountObject(name)`（该名称堆叠总数量）、`GetStackedNames()`（用于显示：数量 >1 时显示“名称 xN”，单个只显示名称）。
  - `GetAllObjectName()` 改为去重后的名称列表（每个堆叠一项），供名称/序号查找。
  - `RemoveObject(name, count=1)`：支持按数量扣减，返回实际移除数；扣到 0 时移除该堆叠并 `delete`。
  - `GetSize()` = 堆叠数（非物品总数）。既有 `GetObjects()` 仍按堆叠返回。
- `src/Tool/src/PlayerSerializer.cpp`：存档物品行追加第 6 段 `|quantity`，`bagCount` 改为堆叠数；读档解析可选第 6 段（旧存档缺省 1）后用 `AddUnique` 还原。
- `main.cpp`：
  - 玩家状态面板改用 `GetStackedNames()`（“1. 小白菜种子 x3”）。
  - `market.sell` 支持堆叠卖出：`GetAllObjectName()` 查找 → `CountObject(name)` 取总量 → `RemoveObject(name, sell_count)` 一次性扣减。

**验证**：新增 `tests/bag/bag_stack_test.cpp`（同名合并、xN 显示、去重、按量扣减、默认移除 1、异名分堆、AddUnique 不合并）。`bag_stack_test` 通过；实际冒烟：买 3 个种子 → 背包显示 `小白菜种子 x3`，卖 2 个 → 剩余 `小白菜种子`。

---

## 阶段 3：简化指令系统 + 数量参数默认值

**修改**：
- `src/View/Cmdparser/src/input_parser.cpp`：`market.buy` / `market.sell` 的 `--count` 参数追加 `->default_str("1")`，省略时取 1；移除不再使用的 `fish.tick --times` 选项。
- `src/View/Cmdparser/include/input_parser.h`：删除 `times_` 成员。
- 各 handler 内 `opt_int(cmd, "count", 1)` 兜底不变。

**验证**：cmdparser 测试全部通过（`cmdparser_syntax_test` 等）。

---

## 阶段 4：钓鱼 / 挖矿改为循环等待 3-6 秒产出，期间不可操作，按 q 退出

### 前台等待循环

`main.cpp` 新增 `wait_action` lambda：随机等待 `kActionWaitMinMs~kActionWaitMaxMs`（3000~6000 毫秒），50ms 步进轮询 `_kbhit()`/`_getch()`（`#include <conio.h>`）；检测到 `q`/`Q` 立即返回 true（并排空同段残留输入，避免污染 REPL 的 `getline`）；自然等待结束返回 false。等待期间游戏世界时间冻结（派发在 `worldMutex` 下执行、后台线程阻塞），属预期设计。

### 钓鱼 `fish.tick`

循环：等待 3-6 秒（可按 q 中止）→ 检查体力 → 扣 `kFishingSatietyCost` → `tickFish()` → 入包 / 加钓鱼经验；q 退出后回到指令模式并汇总“共钓到 n 条鱼”。前置校验不变：必须在海岸、天气允许钓鱼、体力 > 0。

### 挖矿 `mine.start`

不再依赖后台按游戏分钟轮询的会话，改为前台循环：`can_enter(layer, mc)` 前置校验 → 等待 3-6 秒（可按 q 中止）→ `produce_once(layer, mc)` 产出一次 → 入包 / 加采矿经验；矿镐损坏或塌方时提示并中断；q 退出后回到指令模式并汇总“共出矿 n 次”。卸下挖掘现场会话，`mine.stop` / `mine.status` 在新流程下为空转（报告空闲），原 `MiningHandler`/`MiningState` 库与其既有测试保留。

### MiningController 新增公开接口

- `can_enter(layer_id, context)`：层前置条件（等级/照明）预检，无会话副作用。
- `produce_once(layer_id, context)`：即时单次产出（内部 `produce(layer, 1, ... )`）；工具损坏/塌方/空产出返回 `nullopt`。

**验证**：新增矿测 `MIN-CF-011~013`（produce_once 即时产出一次、非法层返回空、can_enter 预检无副作用），`mining_core_flow_test` 通过。交互式按 q 退出的控制台输入路径无法以重定向输入驱动（`_kbhit/_getch` 需真实控制台），以待在交互终端实测。

---

## 阶段 5：优化移动显示——可视化地图与当前位置

**修改**：
- `src/View/Panels/src/MapPanel.cpp`：重写为 ASCII 十字地图（小镇 / 农田 / 小屋 / 海岸 / 矿洞），当前所在地点以 `※` 前缀标记，附“当前位置：xxx”与图例行。
- `main.cpp`：启动欢迎语后即渲染一次地图；每次移动成功后再渲染更新后的地图。

**验证**：新增 `tests/view/panel_render_test.cpp` 两个用例（地图渲染出全部地点与 `※小屋` 标记、仅当前位置带 `※`）。`view_panel_test` 通过；实际冒烟：`move.down` 后地图更新为 `※矿洞`。

---

## 阶段 6：测试与收尾

- 新增测试模块注册：`tests/CMakeLists.txt` 增加 `add_subdirectory(bag)` 与 `add_subdirectory(weather)`。
- 构建：`cmake -B cmake-build-debug -S .` + `cmake --build cmake-build-debug`（CLion 捆绑 MinGW + Ninja）全部目标通过，`MudGame.exe` 正常链接。
- 测试：`ctest --output-on-failure` 共 **24/24 通过**。

| 新增/更新测试 | 覆盖 |
|---|---|
| `bag_stack_test` | 背包堆叠：合并、xN 显示、去重、按量扣减、AddUnique |
| `weather_event_test` | 天气/事件冲突修复：无天气类事件、周五商人、虫害、次日清空 |
| `mining_core_flow_test`（+3 用例） | produce_once / can_enter 前台接口 |
| `view_panel_test`（+2 用例） | 可视化地图与 ※ 当前位置标记 |

- 完成后各阶段报告已按顺序在本文件末尾依次追加。

---

## 阶段 7：所有输入前增加“输入格式”提示

**方案**：View 层新增统一的提示打印原语，组合根按当前位置定制命令格式串，每次 `print_prompt()` 前先输出提示。

**修改**：
- `src/View/include/view_primitives.h` / `src/View/src/view_primitives.cpp`：新增 `print_input_hint(Renderer&, const std::string&)`，输出 `输入格式：<hint>`。
- `main.cpp`：
  - 新增 `input_hint()` lambda：按 `Player::GetPosition()`（小屋/农田/海岸/矿洞/小镇）返回对应命令格式，末尾追加“通用”命令。
  - 新增 `show_prompt()` lambda = `print_input_hint(...)` + `print_prompt()`；替换三处原始 `print_prompt()` 调用（初始提示、空行重提示、每条命令执行后）。
  - 欢迎语改为说明“每次输入前会提示当前位置的指令格式”。

**验证**：新增 `tests/view/panel_render_test.cpp` 用例 `InputHintPrintInputFormatPrefix`（断言输出 `输入格式：…`）。`view_panel_test` 通过；实际冒烟：小屋显示 `【小屋】…`，`move.up` 到小镇后显示 `【小镇】… blacksmith.status | blacksmith.repair …`。

> **补充优化（后续）**：提示改为多行中文——按位置分行列出每条指令并附中文说明（如 `farm.sow --plot <0-N> --crop <…> —— 播种`），末尾附通用指令行（含中文释义）；错误信息改为中文显示：`src/View/Cmdparser/src/input_parser.cpp` 将 CLI11 的 `ParseError` 按类型（缺参 RequiredError / 类型 ConversionError / 数量 ArgumentMismatch / 多余 ExtrasError / 未知选项 OptionNotFound）转译为中文提示，回退时保留原始英文内容。交互与冒烟已验证。

---

## 阶段 8：接入存档 / 读档

**方案**：`PlayerSerializer` 扩展完整存档（玩家 + 金币 + 游戏会话 + 工具状态 + 游戏内时钟总分钟数），组合根接线 `save` / `load` 命令。

**修改**：
- `src/model/Bag/include/Bag.h` / `src/model/Bag/src/Bag.cpp`：补齐深拷贝（复制/移动构造与赋值）。原先仅依赖隐式浅拷贝，`player = Player(...)` 会双重释放背包对象；现复制语义逐对象 `new Object(*obj)`，移动语义转交指针后清空源。
- `src/controller/Tool/include/tool.h` / `src/controller/Tool/src/tool.cpp`：新增 `is_full / set_level / set_durability / repair_ore / repair_ore_count / repair_base_gold / gold_repair_cost / ore_repair_cost`；修复费用 = `repair_base_gold × 损耗占比` 向上取整（满耐久 0、已损耗至少 1），矿石费用 = 金币费用减半向上取整。
- `src/controller/Tool/include/tool_controller.h` / `src/controller/Tool/src/tool_controller.cpp`：代理新增 `max_durability / is_full / repair_full / gold_repair_cost / ore_repair_cost / repair_ore / repair_ore_count / restore(id, level, dur)`（`restore` 夹取到合法范围，供读档还原）。
- `src/Tool/include/PlayerSerializer.h` / `src/Tool/src/PlayerSerializer.cpp`：
  - 完整 `Save/Load(filename, player, game, gold&, tools&, totalGameMinutes&)`：新增 `gold`、`totalPlaySeconds`、`gameTotalMinutes`、工具 `level/durability`（6 键）字段；移除 `std::cout/cerr` 输出残留。
  - 修复既有 bug：物品行终止解析 `buyPrice` 用 `std::getline(buyStr, '|')`——原先吞掉第 6 段 `|quantity`，导致堆叠数量读档恒为 1。
  - 旧存档兼容：缺 `gold` 段保持调用方金币、缺工具段保持调用方工具（不覆盖）。
- CMake：`src/Tool/CMakeLists.txt` 为 `player_serializer` 链接 `tool_controller`；根 `CMakeLists.txt` 将 `player_serializer` 链入 MudGame。
- `src/View/Cmdparser/src/input_parser.cpp`：新增 `load` 子命令。
- `main.cpp`：
  - include `PlayerSerializer.h`，常量 `kSaveFileName = "mudgame.sav"`。
  - `save`：`serializer.Save(kSaveFileName, player, game, gold, tools, timeService.session_total())`。
  - `load`：完整 `Load(...)`；`totalGameMinutes >= 0` 时 `GameDateTime t; t.advance(total); timeService.set_time(t)` 重建游戏内时钟。

**验证**：新增 `tests/player_serializer/player_serializer_roundtrip_test.cpp`（完整字段往返、读档替换背包内容不双重释放、旧存档保持调用方初值）与 `tests/player_serializer/CMakeLists.txt`、`tests/player_serializer` 注册；`tests/bag/bag_stack_test.cpp` 新增复制/移动深拷贝用例。`player_serializer_roundtrip_test`、`bag_stack_test` 通过；实际冒烟：`save` 生成 `mudgame.sav`（含 gold=200、gameTotalMinutes=480、三工具 1 级满耐久），`load` 恢复位置/时间后正常继续。

---

## 阶段 9：铁匠铺——集市内的工具修复服务

**方案**：集市（`Market`）新增 `黑匠(blacksmith)` 商店（`Shop` 标识不挂商品，仅作场所语义），`blacksmith.status` 查看修复信息，`blacksmith.repair --tool <hoe/rod/pickaxe> --method <ore/gold>` 按矿石或金币全额修复。

**修改**：
- `docs/proj.md` 修复配置落库到 `src/controller/Tool/src/tool.cpp` `kToolConfigs`：锄头=黯铁矿×2 / 基准 50，鱼竿=星纹银×2 / 基准 80，矿镐=黯铁矿×2 / 基准 80。
- `src/View/include/dto.h`：新增 `ToolRepairView`（名称/耐久/满耐久/损坏/金币费用/所需矿石/数量/持有数）与 `BlacksmithView`（金币 + 逐工具修复条目）。
- `src/View/Panels/include/BlacksmithPanel.h` / `src/View/Panels/src/BlacksmithPanel.cpp`：渲染金币与每条修复信息；`TerminalView` 新增 `render_blacksmith()` 门面并注入面板；`CMakeLists.txt` 收录新源文件。
- `src/View/Cmdparser/src/input_parser.cpp` / `input_parser.h`：新增 `blacksmith.status` 与 `blacksmith.repair`（`--tool`、`--method` 均必填；新增 `tool_` / `method_` 成员）。
- `main.cpp`：
  - 注册 `Shop("blacksmith", "铁匠铺")` 于 `market`（`grocery` 之后）。
  - `blacksmith.status`：须 `AtTown`，渲染 `make_blacksmith_view()`（金币 + 三工具修复信息，矿石持有数取 `bag.CountObject(repair_ore)`）。
  - `blacksmith.repair`：须 `AtTown`；校验工具 id（hoe/rod/pickaxe）与方式（ore/gold）；`is_full` 直接提示“无需修复”；
    - 矿石方式：`bag.CountObject(ore) < repair_ore_count` 提示不足 → `RemoveObject(ore, n)` + 扣 `ore_repair_cost` 金币；
    - 金币方式：`gold < gold_repair_cost` 提示不足；
    - 通过后 `SetState(Repairing) → repair_full → SetState(Waiting)`，`msg` 反馈。
  - 小镇输入格式提示追加 `blacksmith.status` / `blacksmith.repair ...`。

**验证**：新增 `tests/tool/tool_repair_test.cpp` + `tests/tool/CMakeLists.txt`（费用折算、修复满耐久、矿石配置、restore 夹取、控制器代理）；`tests/view/panel_render_test.cpp` 新增 `BlacksmithPanelRendersRepairInfo`；`tests/cmdparser/semantic/input_parser_semantic_test.cpp` 新增 `blacksmith.*`/`load` 用例。`tool_repair_test`、`view_panel_test`、`cmdparser_semantic_test` 通过；实际冒烟：小镇 `blacksmith.status` 渲染三工具修复条目，满耐久修复提示“完好无损，无需修复”，同目录旧存档 `load` 正常恢复。

---

## 阶段 10：测试与收尾

- 新增测试模块注册：`tests/CMakeLists.txt` 增加 `add_subdirectory(tool)` 与 `add_subdirectory(player_serializer)`。
- 构建：`cmake -B cmake-build-debug -S .` + `cmake --build cmake-build-debug`（CLion 捆绑 MinGW + Ninja）全部目标通过，`MudGame.exe` 正常链接。
- 测试：`ctest --output-on-failure` 共 **26/26 通过**。

| 新增/更新测试 | 覆盖 |
|---|---|
| `bag_stack_test`（+3 用例） | 深拷贝/移动语义：复制、赋值、移动不双重释放 |
| `tool_repair_test` | 修复费用折算、满耐久清零、修复满耐久、矿石配置、restore 夹取、控制器代理 |
| `player_serializer_roundtrip_test` | 完整字段往返、读档替换背包内容、旧存档兼容 |
| `view_panel_test`（+2 用例） | 铁匠铺面板渲染、输入格式提示 |
| `cmdparser_semantic_test`（+3 用例） | save/load、blacksmith.status、blacksmith.repair 必填参数 |

- 完成后各阶段报告已按顺序在本文件末尾依次追加。

---

## 阶段 11：重构输入系统——交互式参数收集

### ① 初始输入仅为行为名称，单独询问每一个参数

**方案**：新增 `CommandSchema`（命令参数定义）与 `ParameterCollector`（交互式参数收集器）。用户输入仅限行为名称（如 `farm.sow`），随后系统对每个参数**单独输出动态提示**并逐项读取输入；不再使用 `--option value` 命令行式输入。

**修改**：
- `src/View/Cmdparser/include/input_parser.h`：
  - 新增 `mud::cmd::ParameterDef`（参数名 / 提示文本 / 是否必填 / 默认值）与 `mud::cmd::CommandSchema`（命令描述 + 参数列表）。
  - 新增 `InputParser::parse_verb_only(line)`：仅从输入行提取首个非空白 token 作为 verb，忽略其余内容（不再要求 `--option` 语法）。
  - 新增 `ParameterCollector` 类：按 Schema 逐个输出提示并读取输入，支持默认值（直接回车）、必填重试、可选跳过（无默认值则回车跳过）、取消（输入 `q`）。
- `src/View/Cmdparser/src/parameter_collector.cpp`：新文件，实现 `ParameterCollector::collect`（默认输入源自动剥离残留 `\r`）。签名兼容 `Renderer` 注入，输出统一走 Renderer。
- `src/View/Cmdparser/include/connector.h` / `src/connector.cpp`：
  - 新增 `register_schema(verb, schema)` / `set_collector` / `has_schema` / `get_schema`。
  - `dispatch` 改为：查 schema → 若有 schema 且 collector 可用 → 调用 `collect` 收集缺失参数 → 再执行 handler；用户取消（输入 q）返回 `Failed` 且不执行 handler。
- `src/View/Cmdparser/CMakeLists.txt`：收录 `parameter_collector.cpp`；PUBLIC 链接 `view_primitives`（获取 `Renderer`）。
- `main.cpp`：
  - 为所有带参命令注册 Schema：`mine.start`（layer）、`time.scale`（factor）、`farm.sow`（plot+crop）、`farm.water`（plot）、`farm.fertilize`（plot+type）、`farm.harvest`（plot）、`market.buy`（shop+item+count[默认1]）、`market.sell`（item+count[默认1]）、`blacksmith.repair`（tool+method）。
  - 动态提示在注册时从游戏状态生成（如 `地块索引(0-3)`、`作物名(cabbage/carrot/tomato/pumpkin/lingzhi)`）。
  - 主循环改用 `parse_verb_only` + Connector 自动收集参数；`connector.set_collector(&paramCollector)` 接线。

### ② 更改相关提示系统

- `main.cpp` 的 `input_hint`：从“列出含 `--option` 的完整语法”改为“仅列出行动名称 + 中文说明”（如 `farm.sow —— 播种`）。
- `help` 展示改为行动名称清单（附参数说明），不再使用 CLI11 生成的带选项帮助文本。
- 欢迎语改为“输入行动名称即可，系统会逐个提示所需参数”。

**验证**：新增 `tests/cmdparser/interactive/interactive_input_test.cpp` 并注册 `cmdparser_interactive_test`（15 用例）：`parse_verb_only`（提取/忽略参数/空白/首 token）、收集器（逐参数、提示文本渲染、默认值、可选跳过、取消、必填重试、跳过已提供选项）、Connector 交互派发（收集后透传 handler、取消不执行 handler、无 schema 直派、`has_schema` 查询）。
`ctest` **27/27 通过**；构建全部目标通过，`MudGame.exe` 正常链接。
冒烟（管道输入真实运行）：`farm.sow` → 逐一提示 `地块索引(0-3)：` / `作物名(...)：` → 播种成功；`market.buy` → 依次提示 shop/item/count → 购入成功；参数处输入 `q` 静默取消并返回提示符；位置门控（不在农田播种提示“你不在农田”“你不在城镇”等）与既有语义一致。