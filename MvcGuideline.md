# 悠闲农场 - MVC 架构规范

## 架构概览

本项目采用 **MVC（Model-View-Controller）** 架构模式，将数据、逻辑、显示三层分离。

```
┌─────────────────────────────────────────────────┐
│                   Controller                     │
│         接收输入 → 调用 Model → 通知 View        │
└───────────┬─────────────────────┬───────────────┘
            │ 读写                │ 读取
            ▼                     ▼
     ┌──────────┐          ┌──────────┐
     │  Model   │          │   View   │
     │ 数据+逻辑 │◄─────── │ 终端渲染  │
     └──────────┘  查询状态 └──────────┘
```

## 三层职责

### Model（数据层）

**职责：** 纯数据存储 + 纯业务逻辑计算，不依赖任何外部系统。

| 原则 | 说明 |
|------|------|
| 存储游戏状态 | 玩家属性、背包、农田、工具等所有运行时数据 |
| 纯逻辑计算 | 生长判定、概率计算、等级计算等 |
| 不感知 UI | 不知道数据如何显示 |
| 不感知输入 | 不知道数据由谁修改 |
| 被动响应 | 只提供接口供 Controller 调用，不主动调用外部 |

**文件位置：** `src/model/`

**包含类：**
- `Player` — 玩家属性（饱食度、金币、三套等级经验）
- `Inventory` — 背包（物品存储、增删查）
- `Farm` — 农田（地块状态、作物生长）
- `TimeSystem` — 时间（昼夜、天数、倍速）
- `Weather` — 天气（当日天气、效果判定）
- `Event` — 随机事件数据

### View（显示层）

**职责：** 纯终端渲染，不包含任何业务逻辑。

| 原则 | 说明 |
|------|------|
| 只做渲染 | 接收数据 → 输出文本到终端 |
| 不修改状态 | 永远不直接修改 Model 数据 |
| 不做判断 | 不包含 if/else 业务逻辑（显示条件除外） |
| 不做计算 | 所有数值由 Model 提供 |
| 单向依赖 | 只读取 Model 数据，不调用 Controller |

**文件位置：** `src/view/`

**包含类：**
- `TerminalView` — 终端界面渲染（菜单、状态面板、战斗文本等）
- `InputParser` — 输入解析（将用户输入转为指令对象）

### Controller（控制层）

**职责：** 连接 Model 和 View，处理所有游戏逻辑。

| 原则 | 说明 |
|------|------|
| 接收输入 | 从 View 获取用户输入 |
| 调用 Model | 读写游戏状态 |
| 通知 View | 将最新状态传给 View 渲染 |
| 包含逻辑 | 所有 if/else、流程控制都在此层 |
| 协调子系统 | 管理状态机、任务进度、事件触发等 |

**文件位置：** `src/controller/`

**包含类：**
- `GameController` — 游戏主循环、初始化、存档
- `StateMachine` — 玩家状态机（行走/种田/钓鱼/采矿）
- `FarmingController` — 种菜操作逻辑
- `FishingController` — 钓鱼操作逻辑
- `MiningController` — 采矿操作逻辑
- `MarketController` — 集市交易逻辑
- `QuestController` — 任务进度逻辑

## 数据流向

### 用户输入处理流程

```
用户按键
  │
  ▼
View.InputParser           ← 解析为指令对象
  │
  ▼
Controller.GameController  ← 分发到对应子控制器
  │
  ▼
Controller.XxxController   ← 执行业务逻辑
  │
  ▼
Model.Xxx                  ← 修改游戏状态
  │
  ▼
Controller.GameController  ← 通知 View 刷新
  │
  ▼
View.TerminalView          ← 重新渲染界面
```

### 每帧循环

```
while (gameRunning) {
    // 1. 时间推进
    timeSystem.tick();

    // 2. 挂机产出检查（钓鱼/采矿）
    stateMachine.tick();

    // 3. 事件检查（08:00）
    eventSystem.check();

    // 4. 天气影响
    weatherSystem.apply();

    // 5. 获取输入
    Input input = view.getInput();

    // 6. 处理输入
    controller.handleInput(input);

    // 7. 渲染
    view.render(model.getState());
}
```

## 目录结构

```
src/
├── main.cpp                    # 入口：初始化 → 启动主循环
│
├── model/                      # 数据层（纯数据+纯逻辑）
│   ├── player.h                # 玩家属性
│   ├── inventory.h             # 背包系统
│   ├── farm.h                  # 农田数据
│   ├── time_system.h           # 时间系统
│   ├── weather.h               # 天气数据
│   └── event.h                 # 事件数据
│
├── controller/                 # 控制层（逻辑+协调）
│   ├── game_controller.h       # 游戏主控制器
│   ├── state_machine.h         # 状态机
│   ├── farming_controller.h    # 种菜逻辑
│   ├── fishing_controller.h    # 钓鱼逻辑
│   ├── mining_controller.h     # 采矿逻辑
│   ├── market_controller.h     # 集市逻辑
│   └── quest_controller.h      # 任务逻辑
│
├── view/                       # 显示层（纯渲染）
│   ├── terminal_view.h         # 终端渲染
│   └── input_parser.h          # 输入解析
│
├── data/                       # 静态配置（常量+表）
│   ├── items.h
│   ├── crops.h
│   ├── fish.h
│   ├── ores.h
│   └── events.h
│
└── util/                       # 工具类
    ├── serializer.h            # JSON 序列化
    └── random.h                # 随机数
```

## 依赖规则

### 硬性约束

```
Model  ──×──► View      # Model 不能引用 View
Model  ──×──► Controller # Model 不能引用 Controller
View   ──×──► Controller # View 不能引用 Controller
View   ──×──► Model      # View 只读取 Model，不主动调用（通过参数传递）
```

### 允许的依赖

```
Controller ──► Model      # Controller 可以读写 Model
Controller ──► View       # Controller 可以调用 View 渲染
View       ◄── 参数       # View 通过函数参数接收数据（不主动查询）
```

### 依赖方向总结

```
Controller ──► Model
Controller ──► View
Model      ◄── Controller（被动被调用）
View       ◄── Controller（被动被调用）
```

## 编码约定

### Model 约定

```cpp
// ✅ 正确：纯数据 + 纯计算
class Player {
public:
    void addExp(ExpType type, int amount);   // 纯逻辑
    int getLevel(ExpType type) const;        // 纯查询
    bool canAct() const;                     // 纯判断

private:
    int satiety_;
    int gold_;
    int farming_exp_;
    // ...
};

// ❌ 错误：Model 中包含 IO 或 UI 逻辑
class Player {
    void printStatus() { cout << "饱食度: " << satiety_; }  // 禁止
    void saveToFile() { /* ... */ }                          // 禁止
};
```

### View 约定

```cpp
// ✅ 正确：只做渲染
class TerminalView {
public:
    void renderGameState(const GameState& state);  // 接收数据渲染
    void showMessage(const string& msg);           // 纯显示
    Input parseInput();                            // 纯解析
};

// ❌ 错误：View 中包含业务逻辑
class TerminalView {
    void render() {
        if (player.satiety < 30)  // 禁止：业务判断
            cout << "你很饿！";
    }
};
```

### Controller 约定

```cpp
// ✅ 正确：协调 Model 和 View
class FarmingController {
public:
    void handlePlant(Player& player, Farm& farm, int plotIndex, CropId cropId) {
        if (!farm.isEmpty(plotIndex)) return;       // 逻辑判断
        if (player.gold < crop.price) return;      // 逻辑判断
        player.spendGold(crop.price);               // 修改 Model
        farm.plant(plotIndex, cropId);              // 修改 Model
        view.showMessage("种植成功！");              // 通知 View
    }
};
```

## 子系统与 MVC 映射

| 子系统 | Model | Controller | View |
|--------|-------|------------|------|
| 时间系统 | TimeSystem | GameController.tick() | 时间面板 |
| 玩家属性 | Player | — (直接操作) | 状态栏 |
| 状态机 | — | StateMachine | 当前行为提示 |
| 地图系统 | LocationId | GameController.move() | 地图菜单 |
| 种菜系统 | Farm, Plot | FarmingController | 农田界面 |
| 钓鱼系统 | FishingSession | FishingController | 钓鱼界面 |
| 采矿系统 | MiningSession | MiningController | 采矿界面 |
| 集市系统 | Market, Shop | MarketController | 商店界面 |
| 工具系统 | Tool | MarketController (修复/升级) | 工具面板 |
| 天气系统 | Weather | GameController | 天气通知 |
| 事件系统 | Event | GameController | 事件弹窗 |
| 任务系统 | Quest, Order | QuestController | 任务列表 |
| 持久化 | — | GameController.save/load | 存档提示 |

---

*文档版本：v1.0*
*创建日期：2026-08-22*
