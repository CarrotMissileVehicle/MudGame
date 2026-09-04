# cmdparser（命令解析器）模块测试说明

## 待测模块边界

被测对象为 `src/View/Cmdparser` 及命令分发相关部件，责任如下（MVC：View → Controller → Model 单向）：

| 组件 | 文件 | 职责 | 是否纳入测试 |
| --- | --- | --- | --- |
| `InputParser` | `input_parser.{h,cpp}` | 输入行 → 结构化 `Command`（点分 verb / 位置参数 / 命名参数） | ✅ 核心 |
| `Command` | `input_parser.h` | 解析结果数据结构 | ✅（作为解析产物） |
| `Connector` | `connector.{h,cpp}` | verb → handler 注册、大小写不敏感路由、缺省 `UnknownCommand` | ✅ 核心 |
| `HandlerContext` | `connector.h` | 透传 `TimeService` 与 `MiningHandler` 运行时依赖 | ➖ 上下文载体 |
| `MiningHandler` | `Mining_controller` | `start / poll / stop` 采矿命令入口（含会话状态门控） | ✅（状态拦截用例） |
| `MiningState` | `Mining_controller` | `Idle ↔ Mining` 会话状态机 | ✅（上下文用例） |
| `TimeService` | `Model/Timeservice` | 时间取时与倍率 | ➖ 仅作依赖，不单独覆盖 |
| 矿石数据 | `Data/Ore/*.json` | ore 种类 / 矿层 / 产出率 | ✅（语义与逻辑数据源） |

**不覆盖**：采矿产量计算细节、时间服务自身行为、其他无关业务模块。

## 现行命令（verb）清单

`InputParser` 声明的子命令（CLI11）：

| verb | 参数 | 说明 |
| --- | --- | --- |
| `mine.start` | `--layer <0-4>`（必填） | 在目标矿层开始采矿 |
| `mine.stop` | — | 停止采矿并结算 |
| `mine.status` | — | 查询采矿状态 |
| `time.now` | — | 查询当前游戏时间 |
| `time.scale` | `--factor <double>`（必填） | 设置时间倍率 |
| `help` | — | 打印帮助 |
| `quit` | — | 退出 |
| `save` | — | 保存 |

## 需求示例指令 → 现行 / 目标形态映射

> 需求示例中的**空格动词**为“目标形态”；现行 schema 为**点分动词**。下表给出两者关系与预期路由，测试据此给出明确预期。

| 需求示例 | 现行等价 verb | 建议断言行为 |
| --- | --- | --- |
| `mine` | `mine.start --layer <L>` | 缺省参数形式应在语义层被要求提供层号；缺参 → `BadArgument` 并提示 |
| `mine copper` / `mine iron` | `mine.start --layer <对应层>`（copper/iron 若在数据中则取其所在层） | 空格式“动词+矿石”在现行 CLI11 下无 `mine` 子命令 → 解析错误 `verb="error"`；目标形态应路由至采矿且校验矿石类型 |
| `MINE COPPER` / `mine copper` | （大小写）`mine.start` 路由 | 标识 Connector 层大小写不敏感；CLI11 子命令匹配大小写需确认 |
| `time` | `time.now` | 现行只有一个 `time.*`；`time` 无子命令 → 解析错误；目标形态应归一为时间查询 |
| `go north` | （未绑定） | 现行未注册 → `UnknownCommand`；目标形态应路由至移动控制器 |
| `look rock` | （未绑定） | 现行未注册 → `UnknownCommand`；目标形态应路由至观察/检查 |

## 矿石种类（测试数据依据）

来自 `Data/Ore/ore.json`（示例指令据矿种判断参考）：

| ore_id | 名称 | 需求等级 | 说明 |
| --- | --- | --- | --- |
| `ore_iron` | 黯铁矿 | 1 | 工具升级材料 |
| `ore_silver` | 星纹银 | 3 | 高级工具材料 |
| `ore_gold` | 赤耀金 | 5 | 贵重品/升级材料 |
| `ore_crystal` | 雷鸣晶 | 8 | 传说工具材料 |
| `ore_core` | 世界之核 | 15 | 至宝/收藏 |

## 玩家状态（上下文用例依据）

来自 `Model/Playerstates` 与采矿会话：

- 眩晕（stun）
- 死亡（dead）
- 采矿中（硬直，`MiningStatus::Mining`）

## 执行前准备（供后续承接自动化）

- 编译目标：`cmd_parser` 库；建议以 GoogleTest/CLI11 单测形式落地。
- 注入桩：对 `MiningHandler`、`TimeService` 提供可控桩以精确断言“后端 handler 是否被调用、以何实参调用”。
- 数据夹具：固定 `ore.json / mining_layers.json` 快照，保证用例可重复。