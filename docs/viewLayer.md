# 胡萝卜山谷 MUD — View 层项目书（TODO 5/6 实施依据）

> 面向"接下来要写代码的人"。本文档是 View 层重构/完善的权威依据，
> 供 TODO 5（重做整个 View 层）与 TODO 6（与 Controller 集成）直接照做。
> 只描述目标态与迁移路径，不修改任何源码。

---

## 0. 一句话目标

把散落在 `main.cpp`（组合根）里的 `std::cout << ...` 直出逻辑**收编**进 `src/View/`，
让 View 变成一套"**只收数据、只做渲染、不查 Model、不判断业务**"的纯渲染组件，
Controller/组合根只负责**装配 DTO → 调 View**，不再逐行手写输出。

---

## 1. 现状盘点

### 1.1 现有 View 组件（可复用/需改造）

| 组件 | 位置 | 能力 | 评价 |
|------|------|------|------|
| `StatusView` + `PlayerStatus` DTO | `src/View/Statusview/` | `ShowStatus(const PlayerStatus&)` 渲染玩家状态面板（位置/状态/饱食度/三系经验/背包），含 `PositionCode`/`StateCode` → 中文名映射 | ✅ **已符合 MVC**（参数注入、纯渲染）。它是很好的样板，**保留并作为其他面板的范式**；命名风格（英文面板/中文内容）可直接沿用。 |
| `InputParser` | `src/View/Cmdparser/` | 基于 CLI11 声明式 schema，`parse(line)` → `mud::cmd::Command`；`help_text()` 生成帮助 | ✅ **已可用**。schema 已覆盖全部 26 条命令。可保留；TODO 5 只需**校验/补齐 schema 与目标命令清单一致**。 |
| `Connector` | `src/View/Cmdparser/` | verb→handler 映射表，`bind()`/`dispatch()`，`HandlerResult` 枚举，`HandlerContext` 携带 `TimeService&` + `MiningHandler&` | ⚠️ **部分越界**：`HandlerContext` 把 `MiningHandler`（业务）直接塞进 View 层类型，且 handler 都是 `main.cpp` 匿名 lambda。TODO 5/6 建议**收窄 Connector 职责**（只做 verb 路由），把具体 handler 上移到 Controller 层。 |

### 1.2 main.cpp 中散落的渲染（需收编清单）

> 放大镜法则：**凡表达式里出现 `std::cout` 且变量只读，说明"本应由 View 渲染"。**

| 位置（main.cpp 行号） | 输出内容 | 应归属面板 | 说明 |
|------|---------|-----------|------|
| L215-218 `print_now` | 当前时间 + 今日天气 | `TimePanel` / `WeatherPanel` | 时间面板 + 天气面板，被 `time.now`/`weather.now`/启动横幅复用 |
| L71-75 `fmt_time` | `Y年M月D日 HH:MM` | `TimePanel` 内部格式化 | 应下沉，形成统一时间格式函数 |
| L185 `grant_mining`（循环内） | 获得矿石 xN (+exp) | `MiningPanel` 产出反馈 | 采矿石产出提示 |
| L222-236 `mine.start` | 不在矿区/层条件不足/开始采矿(层x) | `MiningPanel` + 消息行 | 操作反馈 + 失败消息 |
| L238-248 `mine.stop` | 未采矿/采矿结束 | `MiningPanel` | |
| L250-260 `mine.status` | 采矿中(层 x, 自 t)/空闲 + 采矿等级 | `MiningPanel` | 状态查询 |
| L263-266 `time.now` | print_now | `TimePanel` | |
| L268-273 `time.scale` | 时间倍率设为 x | `TimePanel` | |
| L283-295 `player.status` | 走 `StatusView.ShowStatus(status)` | `StatusPanel` | ✅ 已是正确模式 |
| L299-311 `bind_move` | 不宜外出/移动成功/走不通 | `MessageLine` + 失败消息 | 移动反馈（含 `weather.can_go_outside` 判断为**逻辑**，应留在 Controller） |
| L319-334 `farm.status` | 地块 [i] 作物（生长 a/b，已浇水/缺水）或空地 | `FarmPanel` | 多地块即状态 |
| L340-365 `farm.sow` | 不在农田/未知作物/未解锁/失败/已播种 | `FarmPanel` + 消息行 | 播种反馈（未解锁判断是**逻辑**，留 Controller） |
| L367-382 `farm.water` | 不在农田/失败/已浇水 | `FarmPanel` + 消息行 | |
| L384-406 `farm.fertilize` | 未知肥料/失败/施肥完成 | `FarmPanel` + 消息行 | |
| L408-434 `farm.harvest` | 未成熟/收获 xN (+exp) | `FarmPanel` + 消息行 | 收获反馈 |
| L437-443 `fish.status` | 鱼池 n 种 + 各鱼概率 + 宜钓/不宜 | `FishingPanel` | |
| L445-470 `fish.tick` | 不在海边/不能钓/钓到 X/共钓到 n | `FishingPanel` + 消息行 | |
| L473-486 `weather.now` | print_now + 自动浇水/减产率/采矿经验加成/垂钓减益/今日事件 | `WeatherPanel` | 天气+事件面板 |
| L489-506 `market.status` | 金币/集市日/繁华集/商店货架 | `MarketPanel` | |
| L508-551 `market.buy` | 无此店/无此商品/购买失败/购入 xN(花费) | `MarketPanel` + 消息行 | |
| L553-600 `market.sell` | 无此物品/出售 xN 获得金币 | `MarketPanel` + 消息行 | |
| L603-609 `tools.status` | 工具 耐久/等级/损坏 | `ToolsPanel` | |
| L612-615 `save` | 存档功能尚未接入 | `StatusLine`/消息行 | 占位提示 |
| L618-620 启动横幅 + 提示符 | 欢迎语 + print_now + `> ` | `TerminalView` 启动渲染 | |
| L630-642 主循环 | help 文本 / 再见 / 未知命令 / 提示符 `> ` | `HelpScreen` / `MessageLine` / 提示符 | |

**判定原则**：上述所有输出**只读数据、本身就是显示** → 应下沉到 View。
而每个 `if` 后面的**决策**（`weather.can_fish()`、`crop->getUnlockLevel()` 比较、`valid_plot`、`player.GetPosition()!=AtFarmland`）属于**业务判断**，**必须留在 Controller/组合根**，
只把**判断结果（成功/失败与否、要显示的数据）**汇入 DTO 或消息行传给 View。

---

## 2. View 层目标架构

### 2.1 设计原则（对齐 `docs/MvcGuideline.md`）

```
Controller ──► Model（读写）
Controller ──► View（调 render，传 DTO）
View       ◄── 参数（DTO）
View ──×──► Model      # 不主动查询
View ──×──► Controller # 不反向调用
```

View 层**只做三件事**：收 DTO → 文本拼装 → 输出。
View 层允许"显示条件判断"（如 `bagItems.empty()` 决定打印 `(empty)`），
禁止"业务判断"（如 `satiety<30` 决定内容——那由 Controller 决定传给哪个 DTO/消息）。

### 2.2 建议组件集

按 12 子系统的展示需求，建议分为**两组**：

**A. 通用终端原语 `view_primitives`（纯函数/小工具）**
- `print_separator(width, ch)` — 分隔线（如 `-` × 60）
- `print_title(title, ch)` — 标题栏（上下分隔线夹标题）
- `print_pair(label, value)` — `label: value` 对齐输出
- `format_time(GameDateTime)` — `2026年9月8日 09:05`（收编 `fmt_time`）
- `print_prompt()` — `> `
- 统一走一个 `Renderer` 抽象（见 2.3），默认 stdout

**B. 各面板渲染（每个面板 = 一个类，构造注入 Renderer，接收 DTO）**

| 面板类 | 对应子系统 | 渲染的数据（DTO 字段） | 复用点 |
|--------|-----------|----------------------|--------|
| `TimePanel` | 时间系统 | `TimeView`（时间 + 倍率） | 收编 `fmt_time`/`print_now` |
| `StatusPanel` | 玩家属性/背包 | `PlayerStatus`（现有）+ Gold | 吸收现有 `StatusView` |
| `MapPanel` | 地图系统 | `Position`（当前位置） | — |
| `FarmPanel` | 种菜系统 | `FarmView`（地块数组） | 收编 `farm.status` |
| `FishingPanel` | 钓鱼系统 | `FishingView`（鱼池） | 收编 `fish.status` |
| `MiningPanel` | 采矿系统 | `MiningView`（会话+产出） | 收编 `mine.*` |
| `MarketPanel` | 集市/商店 | `MarketView` | 收编 `market.status` |
| `ToolsPanel` | 工具系统 | `ToolsView` | 收编 `tools.status` |
| `WeatherPanel` | 天气/事件 | `WeatherView` | 收编 `weather.now` |
| `HelpScreen` | 解析/帮助 | 命令清单（来自 InputParser） | 收编 help_text |
| `MessageLine` | — | `std::vector<std::string> msgs`（操作反馈/事件/存档提示） | 统一反馈出口 |

> 说明：`TaskPanel`（任务列表）与 `SavePanel`（存档提示/加载）当前**无对应业务模型**，
> 列为**预留接口**（TODO 后续子系统出现时实现），本次只留 DTO 占位。

**C. 组合视图 `TerminalView`（门面）**
- 持有各面板的 Renderer，提供 `render_all(const GameSnapshot&)` 一次性渲染主界面；
- 以及 `render(panelXXX)` 单面板渲染（供 Controller 按需求单独刷新）。

### 2.3 Renderer 抽象（输出可测试化）

```cpp
// src/View/include/Renderer.h
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void print(const std::string& line) = 0;   // 带换行
    virtual void print_raw(const std::string& s) = 0;  // 不带换行（提示符）
};
class StdoutRenderer : public Renderer { /* std::cout */ };
class StringRenderer  : public Renderer { /* 累积进 string（给单元测试用）*/ };
```

> **收益**：所有面板只依赖 `Renderer&`，可注入 `StringRenderer` 做 GoogleTest 断言，
> 满足"View 层亦可单测"的目标（参考 `tests/` 既有 cmdparser 测试的风格）。

### 2.4 InputParser（schema）与 Connector（dispatch）职责边界

- **`InputParser`**：只负责"行 → `Command`"的**语法层**（verb/args/options）。
  命令 schema 是**描述性输入契约**，留在 View 层合理 ✅。
- **`Connector`**：只做 **verb → handler 查表路由**（`find`/`dispatch`）。
  handler 的注册**上移到 Controller/组合根**，`Connector` 不再感知业务类型。
  - 建议：`HandlerContext` 里 `MiningHandler&` 这类业务依赖**移出** View 类型，
    改为 handler 通过**闭包捕获**（现状 `main.cpp` lambda 已天然闭包捕获）——即
    **删掉 `HandlerContext`，让 handler 签名只接 `const Command&`**（详见 §6）。
  - `HandlerResult` 枚举保留（Ok/BadArgument/Failed/UnknownCommand 仍有用）。

---

## 3. View 层与 Controller 的数据契约（DTO 设计）

> 铁律：**所有 DTO 都是只读**（`const` 字段 / POD 结构 / `const&` 视图），
> **由 Controller/组合根装配**，**View 只读**。DTO 放 `src/View/include/dto.h`。

### 3.1 通用 / 面板 DTO

```cpp
// 见 StatusView.h —— 保留现有结构（吸收为 StatusPanel 输入）
struct PlayerStatus {
    PositionCode position; StateCode state;
    int satiety; int maxSatiety;
    int farmingExp; int fishExp; int mineExp;
    std::vector<std::string> bagItems;   // Bag::GetAllObjectName()
};

// 新增（收编 print_now / fmt_time）
struct TimeView {
    int year, month, day, hour, minute; // timeService.now()
    double time_scale;                  // timeService.time_scale()
    static TimeView from(const mud::time::GameDateTime&, double scale);
};

struct WeatherView {
    std::string weather;            // weather.weather_name()
    bool can_fish, can_go_outside;  // weather.can_fish() / can_go_outside()
    bool auto_water;                // weather.auto_water()
    double crop_loss_rate;          // weather.crop_loss_rate()
    double mining_exp_bonus;        // weather.mining_exp_bonus()
    double fishing_penalty;         // weather.fishing_penalty()
    std::vector<std::string> events;// weather.today_event_names()
};

struct PlotView {
    std::size_t index;
    bool occupied;
    std::string crop_name;   // kCropNames 映射；空地=""
    int growth_stage, growth_max; // FarmLand::getGrowthStage() / getCrop()->getGrowthCycle()
    bool watered;
};
struct FarmView {
    std::vector<PlotView> plots;   // for i in farm.size()
};

struct FishEntry { std::string name; float probability; }; // Fish::getProbability()
struct FishingView {
    std::vector<FishEntry> pool; // kFishPool
    bool can_fish;               // weather.can_fish()
};

struct MarketItemView { std::string name; int buy; int sell; }; // getBuyPrice/getSellPrice
struct ShopView { std::string id; std::string name; std::vector<MarketItemView> items; };
struct MarketView {
    int gold;                    // 组合根 gold
    int day_of_week;             // market.getDayOfWeek()
    bool prosperous, festival;   // isProsperousDay() / isFestival()
    std::vector<ShopView> shops; // market.getShop(i) 遍历
};

struct ToolView { std::string name; int durability; int level; bool broken; };
struct ToolsView { std::vector<ToolView> tools; }; // Hoe/Rod/Pickaxe

struct MiningView {
    bool is_mining;
    std::size_t layer;           // *miningHandler.layer_id()
    mud::time::GameDateTime start_time;
    std::size_t mining_level;    // 1 + player.GetMineExp()/100
};
struct MiningEventView {         // 产出入库反馈（grant_mining 输出）
    std::string ore_name;
    std::size_t quantity;
    std::size_t experience;
};

// 操作反馈：Controller 把"这次动作结果文本"作为纯字符串交给 View，View 只负责展示。
using MessageLine = std::vector<std::string>;
```

### 3.2 聚合快照 `GameSnapshot`（供 `TerminalView::render_all`）

```cpp
struct GameSnapshot {
    TimeView time;
    PlayerStatus player;   // 含位置
    WeatherView weather;
    FarmView farm;
    FishingView fishing;
    MarketView market;
    ToolsView tools;
    MiningView mining;
    MessageLine messages;  // 待滚动显示的反馈行
};
```

> Controller/组合根在每次需要全量刷新时**装配一份快照 → `terminal.render_all(snap)`**；
> 单命令反馈只调 `MessageLine`/对应面板，不必每次都全量。

### 3.3 数据来源对照表（DTO 字段 ← 谁提供）

| DTO 字段 | 提供者（Model/Controller） |
|---------|--------------------------|
| `TimeView` | `mud::TimeService::now()/time_scale()` |
| `PlayerStatus.position/state/satiety/exp` | `Player::GetPosition/GetState/GetSatiety/GetFarmingExp/...` |
| `PlayerStatus.bagItems` | `Player::GetBag().GetAllObjectName()` |
| `WeatherView.*` | `WeatherController::weather_name()/can_fish()/auto_water()/...` |
| `PlotView` | `Farm::getFarmland(i)` + `Crop::getGrowthCycle()` + `kCropNames` |
| `FishingView` | `FishingController::poolSize()` + `Fish::getProbability()` + `kFishNames` |
| `MarketView` | `Market::getDayOfWeek()/isProsperousDay()/isFestival()/getShop(i)/getBuyPrice()/getSellPrice()` |
| `ToolsView` | `mud::tool::ToolController::name()/durability()/level()/is_broken()` |
| `MiningView` | `MiningHandler::is_mining()/layer_id()/start_time()` + `Player::GetMineExp()` |

---

## 4. 渲染与中文交互规范

1. **UTF-8 编码**：所有字符串按 UTF-8 输出。源码文件保持 `/utf-8`（MSVC）或 UTF-8 源编码；中文面板文本与现有 `StatusView` 一致。
2. **终端宽度**：面板内容控制在 **80 列**内；长分隔线宽取 `60` 字符固定值（`-`/`=`），标题两端对称。
3. **面板标题风格**：沿用现有 `StatusView` 的包裹式标题：
   ```
   ========== Farm ==========
   ...
   ==================================
   ```
   建议统一为 `print_title(title, '=')` 生成。
4. **分隔线**：`print_separator('-', 33)`（与现有 `ShowStatus` 内部 `---` 一致）。
5. **无 ANSI / 最小可用**：**默认不引入** ANSI 转义序列（跨平台、测试友好）。如需高亮（如损坏工具 `[已损坏]`），用文本标记而非颜色码；RFC 明确"无 ANSI 或最小可用"。
6. **只做显示判断**：空背包 → `(empty)`、空地块 → `空地`、空事件 → 不打印事件行。这些是"有无数据显示"的显示判断，允许。
   **禁止业务判断**：不出现 `if (satiety<30)`、`if (x > unlock)` 之类——它们决定"要不要渲染/渲染什么内容"属于 Controller 职责。
7. **数值一律来自 DTO**：View 内不做任何 `+ * /` 计算、不拼装价格、不推导等级。所有数字已是 DTO 字段。
8. **命名空间与风格**：新 DTO 与面板可放入 `mud::view` 命名空间；沿用现有 `PlayerStatus`（全局空间）不做强制迁移，但新代码建议归入 `mud::view` 以免全局污染。
9. **编码格式**：方法与现有 `ShowStatus` 一致——render 方法接收 `const DTO&` 参数，内部只 `renderer.print(...)`。

---

## 5. 文件 / 目录规划

> 磁盘沿用现有大小写 `src/View/`（与 git 一致，符合 AGENTS.md 说明）。
> 每个新模块 = 独立 `CMakeLists.txt`，**显式列出源文件**（禁 globbing）。

```
src/View/
├── include/                     # 跨面板公共头（DTO + 渲染接口）【新】
│   ├── dto.h                    # 全部只读 DTO（§3）
│   ├── Renderer.h               # Renderer 抽象 + Stdout/String 实现【新】
│   └── view_primitives.h        # 分隔线/标题/时间格式化声明【新】
├── src/
│   ├── Renderer.cpp             # Stdout/String Renderer【新】
│   └── view_primitives.cpp      # 原语实现（收编 fmt_time）【新】
├── Panels/                      # 各面板（每个面板 1 类）【新】
│   ├── include/TimePanel.h, StatusPanel.h, MapPanel.h, FarmPanel.h,
│   │         FishingPanel.h, MiningPanel.h, MarketPanel.h, ToolsPanel.h,
│   │         WeatherPanel.h, HelpScreen.h, MessageLine.h, TerminalView.h
│   ├── src/   (同名 .cpp)
│   └── CMakeLists.txt           # add_library(view_panels src/*.cpp ...)
├── Statusview/                  # 现有【保留→吸收进 Panels/StatusPanel】
│   ├── include/StatusView.h
│   └── src/StatusView.cpp
└── Cmdparser/                   # 现有【保留】(InputParser/Connector)
    ├── include/input_parser.h, connector.h
    ├── src/input_parser.cpp, connector.cpp
    └── CMakeLists.txt
```

- **Statusview 合并策略**：新建 `Panels/StatusPanel`（构造注入 Renderer、接收 `PlayerStatus`），
  把现有 `StatusView::ShowStatus` 的实现迁入，**删除 `Statusview` 模块**（或保留兼容壳）。
  建议直接迁移，避免两个"玩家状态渲染"并存。
- **Cmdparser 复用策略**：`InputParser` 原样保留；`Connector` 保留路由能力，**移除对业务类型
  （`MiningHandler`）的直接依赖**（见 §6）。
- **CMake 接线**：根 `src/CMakeLists.txt` 增加 `view_panels` 与 `Renderer/view_primitives` 目标；
  `MudGame.exe` 链接新增 View 库。

---

## 6. 与 Controller 集成建议

### 6.1 构造注入

组合根（`main.cpp`）装配：
```cpp
StdoutRenderer renderer;              // 唯一输出端
TerminalView terminal(renderer);      // 门面，内部持有各面板
```
各 Controller 若需渲染，通过**构造函数注入 `TerminalView&`（或对应面板的 Renderer&）**，
符合 MVC 规约"Controller → View 调用 render"。

### 6.2 渲染时机

- **动作后**：单命令 handler 返回前，直接 `terminal.render_time(...)` /
  `terminal.message(lines)` 反馈。
- **全量**：启动、`render_all(snap)`；后台时间自动推进期间可按需调用 `render_time` + 相关面板。
- 主循环仅负责 `print_prompt()` 与 `getline`。

### 6.3 事件 / 通知传递

- **操作反馈（消息行）**：Controller 把结果文本 `std::vector<std::string>` 交给
  `terminal.message()`（对应现 `std::cout << "..."` 各处）。
- **播放 / 挂机产出（采矿、钓鱼 tick 结算）**：`MiningPanel` / `FishingPanel` 通过
  DTO 接收产出列表，Controller 每次结算后 `terminal.message(mining_events)`。
- **定时事件（如 08:00、跨天）**：由 Controller 在 `tick_world` 阶段判定后调用
  `terminal.message(...)`——**事件是否触发是业务，触发后显示是 View**。

### 6.4 Connector 收窄

```
现状：  line → InputParser → Command → Connector(Handler with HandlerContext{TimeService&, MiningHandler&})
目标：  line → InputParser → Command → Connector(Handler(const Command&))   # 业务依赖由闭包捕获
```
- 删 `HandlerContext`（或清空其业务字段）；handler 改为 `std::function<HandlerResult(const mud::cmd::Command&)>`；
- `main.cpp` 各 handler lambda 已闭包捕获 `timeService`/`farming`/`miningHandler`/`terminal`，移除对 `ctx` 的依赖即可；
- `Connector` 仍滞留 View 层但只做"verb→回调"查表，不再关心业务类型，符合职责。

### 6.5 与 TODO 6 的边界

TODO 6 集成时，`main.cpp` 从"组合根手写 cout"收敛为"组合根：
**装配 DTO → 调 terminal render（成败由 handler 返回 `HandlerResult` 决定要不要提示 + 用哪个 DTO）**"。
所有 `std::cout` 从 `main.cpp` 移除，只留 `StdoutRenderer` 一处 IO 出口。

---

## 7. 编码约定与 Checklist（TODO 5/6 验收标准）

### 7.1 View 层**禁止事项**

- ❌ 读写 Model（不 include/调用 `Player`/`Farm`/`Market`/`TimeService` 等业务类；DTO 是唯一输入）。
  - 例外：`Renderer`/`dto.h` 可 include `PositionCode.h`/`PlayerStateCode.h`、`GameDateTime`（纯类型别名，非业务修改）。
- ❌ 调用 Controller / 反向依赖。
- ❌ 业务 `if/else`（`satiety<30`、等级比较、`can_fish` 决策、解锁判定）。
- ❌ 业务计算（拼价、推等级、累加经验、`x/y` 生长进度除法若需展示也应在 DTO 中算好）。
- ❌ 直接 `std::cout`（一律走 `Renderer`）。
- ❌ 不显式声明源文件的 CMake 添加（改 `CMakeLists.txt` 必须逐项列出）。

### 7.2 View 层**允许事项**

- ✅ 只读 DTO 结构的定义与字段读取。
- ✅ `Render` 类方法接收 `const DTO&`。
- ✅ 显示条件判断（空容器 → `(empty)` / `空地`）。
- ✅ 枚举 → 中文显示名映射（现有 `GetPositionName`/`GetStateName` 式的映射表）。
- ✅ 文本排版、对齐、分隔线、标题。
- ✅ `InputParser` 语法解析、`Connector` verb 路由（纯路由，无业务）。

### 7.3 验收标准（TODO 5/6 完成后）

1. **`grep -n "std::cout" main.cpp` 为空**（或仅剩组合根装配 `StdoutRenderer`），所有用户可见文本由 View 产出。
2. **`main.cpp` 中不再直接访问 `Player::GetBag()`/`Farm::getFarmland`/`market.getShop` 等做渲染循环**——循环与拼装迁入对应面板的 DTO 装配处。
3. **View 层源文件不含业务 include**（grep 无 `Player.h/Farm.h/Market.h/TimeService.h` 于 `src/View/Panels`，`dto.h/Renderer` 除外）。
4. **全命令可期**：26 条命令的反馈文本与现有行为一致（能对照旧输出回归）。
5. **可测试**：`views` 相关单测存在（用 `StringRenderer` 断言面板输出），`ctest` 全绿。
6. **编译**：`cmake -B cmake-build-debug -S . && cmake --build cmake-build-debug` 通过。
7. **MVC 依赖方向**：无 `Model→View`、`View→Controller` 双向/反向依赖。
8. **中文面板**：所有面板标题为中文（如 `========== 农田 ==========`），内容为中文；时间格式 `Y年M月D日 HH:MM`。

---

## 附 A：预期输入命令表（View 帮助文本 / 解析 schema 依据）

> 来自 `InputParser` 实际注册的 26 条命令 + `main.cpp` handler 语义。
> 供 `HelpScreen` 帮助文本与后续 schema 维护共用。

| # | 命令 | 参数 | 说明 | 示例 |
|---|------|------|------|------|
| 1 | `help` | — | 显示本帮助 | `help` |
| 2 | `quit` | — | 退出游戏 | `quit` |
| 3 | `save` | — | 保存进度（尚未接入） | `save` |
| 4 | `time.now` | — | 查看当前时间 | `time.now` |
| 5 | `time.scale` | `--factor〈必填〉倍率` | 设置时间倍率（时间随真实时间自动流逝） | `time.scale --factor 120` |
| 6 | `player.status` | — | 查看玩家状态 | `player.status` |
| 7 | `move.up` | — | 向上移动 | `move.up` |
| 8 | `move.down` | — | 向下移动 | `move.down` |
| 9 | `move.left` | — | 向左移动 | `move.left` |
| 10 | `move.right` | — | 向右移动 | `move.right` |
| 11 | `farm.status` | — | 查看农田各地块状态 | `farm.status` |
| 12 | `farm.sow` | `--plot〈必填〉地块`、`--crop〈必填〉作物` | 播种 | `farm.sow --plot 0 --crop cabbage` |
| 13 | `farm.water` | `--plot〈必填〉地块` | 浇水 | `farm.water --plot 0` |
| 14 | `farm.fertilize` | `--plot〈必填〉`、`--type〈必填〉normal/advanced` | 施肥 | `farm.fertilize --plot 0 --type normal` |
| 15 | `farm.harvest` | `--plot〈必填〉地块` | 收割 | `farm.harvest --plot 0` |
| 16 | `fish.status` | — | 查看鱼池与可钓性 | `fish.status` |
| 17 | `fish.tick` | `--times〈默认1〉次数` | 尝试垂钓 | `fish.tick --times 5` |
| 18 | `weather.now` | — | 查看今日天气与事件 | `weather.now` |
| 19 | `market.status` | — | 查看集市行情 | `market.status` |
| 20 | `market.buy` | `--shop〈必填〉seed/grocery`、`--item〈必填〉物品`、`--count〈默认1〉` | 购买物品 | `market.buy --shop seed --item 小白菜种子 --count 2` |
| 21 | `market.sell` | `--item〈必填〉物品/序号`、`--count〈默认1〉` | 出售背包物品 | `market.sell --item 胡萝卜 --count 3` |
| 22 | `tools.status` | — | 查看工具耐久与等级 | `tools.status` |
| 23 | `mine.start` | `--layer〈必填〉层` | 开始采矿（0-4） | `mine.start --layer 0` |
| 24 | `mine.stop` | — | 停止采矿 | `mine.stop` |
| 25 | `mine.status` | — | 查看采矿状态 | `mine.status` |

### 参数速查（命名可选值）
- `--crop`：`cabbage|carrot|tomato|pumpkin|lingzhi`
- `--type`：`normal|advanced`
- `--shop`：`seed|grocery`
- `--layer`：`0`(浅层) `1`(中层) `2`(深层) `3`(水晶) `4`(核心)
- `--item`：可用物品名，**或**背包/货架序号（`market.sell`/`market.buy` 支持数字索引）

---

*文档版本：v1.0*
*创建日期：2026-09-08*
*基于现有 src/View、main.cpp、docs/MvcGuideline.md、docs/subsys.md、docs/模板总结.md 生成*
