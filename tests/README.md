# MUD 游戏测试目录总览

本目录按**模块 → 测试维度**组织 MUD 项目的测试方案、用例设计文档与自动化测试代码，遵循**模块 → 细分维度**的分层约定。
目前已落地三大模块：命令解析器（`cmdparser`）、采矿玩法（`mining_controller`）、游戏时间管理（`timeservice`）。
每模块下再按各自维度细分：`cmdparser` 按**语法/语义/上下文/逻辑**四层，`mining_controller` 与 `timeservice` 按**测试方向**。

## 目录结构

```
tests/
├── README.md                     # 本页：总览、分层约定、通用通过标准、可执行目标、风险提示
├── CMakeLists.txt                # 聚合全部 GoogleTest 可执行目标，注册进 CTest
├── cmdparser/                    # 命令解析器：语法/语义/上下文/逻辑四层
│   ├── README.md                 # verb 清单、需求→现行/目标形态映射、数据/状态依据
│   ├── syntax/    cases.md + input_parser_syntax_test.cpp    # CP-S  语法级
│   ├── semantic/  cases.md + input_parser_semantic_test.cpp  # CP-M  语义级
│   ├── context/   cases.md + state_gate_test.cpp             # CP-C  上下文级
│   └── logic/     cases.md + connector_dispatch_test.cpp     # CP-L  逻辑级
├── mining_controller/            # 采矿玩法：按测试方向细分
│   ├── README.md                 # 模块范围、业务链路、通用判定标准
│   ├── precondition/     cases.md + precondition_test.cpp     # MIN-PR 前置条件
│   ├── core_flow/        cases.md + core_flow_test.cpp        # MIN-CF 核心流程
│   ├── probability/      cases.md + probability_test.cpp      # MIN-PB 掉落概率
│   ├── economy_bag/      cases.md + economy_bag_test.cpp      # MIN-EC 经济与背包
│   ├── transaction/      cases.md + transaction_test.cpp      # MIN-TX 事务一致性
│   ├── resource_refresh/ cases.md + resource_refresh_test.cpp # MIN-RF 资源刷新
│   ├── concurrency/      cases.md + concurrency_test.cpp      # MIN-CC 并发抢矿
│   └── boundary/         cases.md + boundary_test.cpp         # MIN-BD 边界异常
└── timeservice/                  # 游戏时间管理：按能力方向细分（纯模型服务）
    ├── README.md                 # TimeService/GameDateTime 接口口径、风险清单 R1~R7
    ├── tick_engine/      cases.md + tick_test.cpp             # TS-TE Tick/心跳
    ├── time_scale/       cases.md + scale_test.cpp            # TS-SC 时间倍率
    ├── precision/        cases.md + precision_test.cpp        # TS-PR 精度/漂移
    ├── calendar_event/   cases.md + calendar_test.cpp         # TS-CE 日历/事件
    ├── timer_queue/      cases.md + timer_test.cpp            # TS-TQ 定时器队列
    ├── cancel_reset/     cases.md + cancel_test.cpp           # TS-CR 取消/重置
    ├── clock_anomaly/    cases.md + clock_anomaly_test.cpp    # TS-CA 时钟异常
    └── load_concurrency/ cases.md + load_test.cpp             # TS-LC 并发/负载
```

> 每个维度目录统一放两类文件：`cases.md`（用例设计与判定口径，人类可读）+ `<xxx>_test.cpp`（GoogleTest 可执行实现）。
> 模块总览见各子目录 `README.md`；此处为跨模块总览。

## 模块分工与细分维度

| 模块 | 被测对象（MVC） | 细分维度 | 维度说明 |
| --- | --- | --- | --- |
| `cmdparser` | `src/View/Cmdparser`（View） | 语法级 | 文本如何被切分与规范化：分词、空白、大小写、缩写别名、异常字符 |
| | | 语义级 | 参数值是否正确解释：必填/可选参数、取值范围、类型、资源/指令语义 |
| | | 上下文级 | 玩家/全局状态是否拦截命令：眩晕、死亡、采矿中（硬直）、权限门槛 |
| | | 逻辑级 | 命令是否路由到正确后端并被环境校验：分发命中、后端调用、作用域/资源/方向 |
| `mining_controller` | `src/Controller/Mining_controller`（Controller） | 前置条件 | 工具存在/耐久、背包、矿脉储量、玩家状态、照明/等级 |
| | | 核心流程 | 读条、BUSY 锁定、状态机 Idle↔Mining、产出结算 |
| | | 掉落概率 | 分布校验、大样本统计验证 |
| | | 经济与背包 | 容量、负重、掉落、邮件补发 |
| | | 事务一致性 | 储量-1 / 耐久-1 / 矿石+1 原子回滚 |
| | | 资源刷新 | 采空提示、刷新触发、重复刷新、跨服同步 |
| | | 并发抢矿 | 锁机制、防超额产出、防重复扣减 |
| | | 边界异常 | 高频外挂、畸形输入、离线、切图 |
| `timeservice` | `src/Model/Timeservice`（Model） | Tick/心跳 | 帧推进、亚分钟进位、无磁越帧 |
| | | 时间倍率 | 倍率换算、非整倍率、中途变速 |
| | | 精度/漂移 | 长时间运行精度、累计收敛 |
| | | 日历事件 | 日历进位、昼夜、季节/月界/年界、全局事件 |
| | | 定时器队列 | 一次性/周期定时、到期触发精度、同帧多回调 |
| | | 取消/重置 | 取消、重置、玩家打断、防幽灵结算 |
| | | 时钟异常 | 系统时钟前跳/后跳、时间倒流、NTP 修正 |
| | | 并发/负载 | 高并发、大批量定时、瞬时洪峰 |

> `timeservice` 为**纯模型服务**，无命令解析入口与会话上下文，故不采用四层分层，而按能力维度组织（命名 `mud::time::GameDateTime` 显式日历字段、最小粒度=分钟、帧驱动唯一时间源）。

## 可执行测试目标（CTest）

`tests/CMakeLists.txt` 依据被测库（`cmd_parser` / `time_service` / `mining_controller`）分组注册以下 GoogleTest 可执行目标。

### cmdparser（链接 `cmd_parser`）

| CTest 目标 | 对应文件 | 层级 |
| --- | --- | --- |
| `cmdparser_syntax_test` | `cmdparser/syntax/input_parser_syntax_test.cpp` | CP-S |
| `cmdparser_semantic_test` | `cmdparser/semantic/input_parser_semantic_test.cpp` | CP-M |
| `cmdparser_context_test` | `cmdparser/context/state_gate_test.cpp` | CP-C |
| `cmdparser_logic_test` | `cmdparser/logic/connector_dispatch_test.cpp` | CP-L |

### timeservice（链接 `time_service`）

| CTest 目标 | 对应文件 | 方向 |
| --- | --- | --- |
| `timeservice_tick_test` | `timeservice/tick_engine/tick_test.cpp` | TS-TE |
| `timeservice_scale_test` | `timeservice/time_scale/scale_test.cpp` | TS-SC |
| `timeservice_precision_test` | `timeservice/precision/precision_test.cpp` | TS-PR |
| `timeservice_calendar_test` | `timeservice/calendar_event/calendar_test.cpp` | TS-CE |
| `timeservice_timer_test` | `timeservice/timer_queue/timer_test.cpp` | TS-TQ |
| `timeservice_cancel_test` | `timeservice/cancel_reset/cancel_test.cpp` | TS-CR |
| `timeservice_clock_anomaly_test` | `timeservice/clock_anomaly/clock_anomaly_test.cpp` | TS-CA |
| `timeservice_load_test` | `timeservice/load_concurrency/load_test.cpp` | TS-LC |

### mining_controller（链接 `mining_controller`）

| CTest 目标 | 对应文件 | 方向 |
| --- | --- | --- |
| `mining_precondition_test` | `mining_controller/precondition/precondition_test.cpp` | MIN-PR |
| `mining_core_flow_test` | `mining_controller/core_flow/core_flow_test.cpp` | MIN-CF |
| `mining_probability_test` | `mining_controller/probability/probability_test.cpp` | MIN-PB |
| `mining_economy_bag_test` | `mining_controller/economy_bag/economy_bag_test.cpp` | MIN-EC |
| `mining_transaction_test` | `mining_controller/transaction/transaction_test.cpp` | MIN-TX |
| `mining_refresh_test` | `mining_controller/resource_refresh/resource_refresh_test.cpp` | MIN-RF |
| `mining_concurrency_test` | `mining_controller/concurrency/concurrency_test.cpp` | MIN-CC |
| `mining_boundary_test` | `mining_controller/boundary/boundary_test.cpp` | MIN-BD |

## 用例编号规范

统一采用 `MOD-POS-SEQ` 三段式编号：

- `MOD`：模块缩写（命令解析器 = `CP`、采矿玩法 = `MIN`、时间管理 = `TS`）。
- `POS`：
  - `cmdparser` 层级代码：语法 `S`、语义 `M`、上下文 `C`、逻辑 `L`。
  - `mining_controller` 方向代码：前置 `PR`、核心流程 `CF`、概率 `PB`、经济背包 `EC`、事务 `TX`、刷新 `RF`、并发 `CC`、边界 `BD`。
  - `timeservice` 方向代码：tick `TE`、倍率 `SC`、精度 `PR`、日历 `CE`、定时器 `TQ`、取消 `CR`、时钟异常 `CA`、并发负载 `LC`。
- `SEQ`：三位序号，如 `001`。

> 注意：`PR` 在 `MIN` 与 `TS` 两模块中含义不同（采矿=前置条件、时间=精度），因 `MOD` 前缀不同故不冲突，但阅读时务必携带模块前缀。

示例：`CP-S-001`、`CP-M-012`、`CP-C-003`、`CP-L-008`、`MIN-PR-002`、`MIN-CC-003`、`TS-TE-001`、`TS-CR-005`。

## 通用通过 / 失败判定标准

对每类测试按以下指标判定（具体到用例另附预期）：

1. **解析结果（语法）**：返回的 `Command`（verb / args / options / raw）与预期一致；空行返回空 verb；解析错误返回 `verb="error"`；`--help` 返回 `verb="help"`。
2. **返回值 / 提示（语义与上下文）**：`HandlerResult`（`Ok / UnknownCommand / BadArgument / Failed`）及面向玩家的反馈文本符合预期。
3. **后端调用是否发生（逻辑）**：用 handler 是否被调用 + 调用实参判定错误路径是否被正确拦截（错误路径上 handler **必须不得**被调用）。
4. **状态/副作用守恒（采矿/时间）**：状态机仅在合法路径跃迁（`Idle ↔ Mining`）；一次合法轮询/结算只产生一次「储量 -1、耐久 -1、矿石 +1」；取消后定时器不得触发（防幽灵结算）。
5. **时长换算口径（时间）**：`now()` 与 `session_total()` 在给定帧数/倍率下的期望值一致，期望须用独立公式重算；回调仅在 `(before, after]` 窗口内触发且仅一次。
6. **日志记录**：关键命令（启动/停止/异常/拦截）、时间推进跨界、触发/取消/变速产生可校验的日志条目。
7. **异常处理**：任何输入（超长、注入、Emoji、越界、`time_scale<0`、空回调）均不导致崩溃、不产生未捕获异常、不误分发、不污染状态。
8. **统计合规 / 并发安全**：大样本掉落分布与 `spawn_rates.json` 一致；多人抢同一低储量矿脉时总产出不超初始储量。

## 风险提示

- 当前命令 schema 为**点分动词**（如 `mine.start --layer 2`），非空格动词形式；需求中提到的空间指令（`mine iron` / `go north` / `time` …）属于**目标形态**，与现行 verb 的映射见 `cmdparser/README.md`。
- CLI11 的子命令大小写匹配行为与 Connector 层的大小写不敏感路由**不在同一层**，相关用例已标注"需确认"。
- 别名（`m`→`mine`、`n`→`north`）当前未实现，相关用例标注为"仅当系统支持别名时执行"。
- 采矿模块的**储量/耐久/背包容量/数据库事务/锁机制/资源刷新/邮件补发**当前尚未落地，相关用例标记为「目标形态」按设计验收。
- 时间模块 `timeservice/README.md` 列出的关键风险 `R1~R7` 需逐条核实：`R1~R5`（幽灵结算、倒流、跳跃丢事件、周期漂移、非整倍率精度）直接针对当前代码；`R6`（日历全局事件）、`R7`（NTP 跳变自修正）为需求目标形态，当前未实现，用例标记为 `P3 / 扩展`，实现后回归执行。
- 时间模块以**帧驱动**为唯一时间源（外部游戏循环调 `update()`），非真实墙钟驱动；"到期触发误差 ≤ 50ms"仅在真实时钟/墙钟驱动形态下适用，当前以"目标帧内不跨帧"为准。
- 本目录为测试**方案、用例设计文档与可执行代码**的聚合；新增测试方向时，请同步在 `tests/CMakeLists.txt` 注册对应 CTest 目标，并在本总览的目录结构与目标表中补充登记。