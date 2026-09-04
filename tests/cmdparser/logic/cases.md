# 逻辑级测试用例（logic）

> 关注：`Connector` 是否正确路由、环境作用域/资源/方向校验是否在正确时机拦截，
> 以及“后端 handler 是否被调用、以何实参调用”。
> 判定基准：见 `../README.md`「通用通过 / 失败判定标准」。

## CP-L 用例清单

### CP-L-001 已绑定 verb 正确派发
- **前置**：`Connector.bind("mine.start", stub)` 绑定探针 handler。
- **输入**：`mine.start --layer 1`
- **步骤**：`dispatch` 到探针，断言返回值与传入实参。
- **预期**：返回 `Ok`；探针被恰好调用一次，收到的 `Command.verb == "mine.start"`。
- **验证重点**：命中路由、实参原样透传、返回码上行。

### CP-L-002 未绑定 verb → UnknownCommand 且不调用后端
- **前置**：未绑定 `go`/`look`/任意未知 verb；已绑定探针用于对照。
- **输入**：`go north`、`look rock`、`foo bar`
- **预期**：`Connector` 返回 `UnknownCommand`；**任何 handler 均不被调用**（探针计数为 0）。
- **验证重点**：未知动词不触发现有后端逻辑（后端调用不发生）。

### CP-L-003 空 verb 不路由
- **前置**：`parse("")` 返回空 verb。
- **输入**：空行。
- **预期**：`dispatch` 对空 verb 不命中任何 handler，不产生调用；返回 `UnknownCommand` 或约定结果。
- **验证重点**：空命令零副作用。

### CP-L-004 Connector 大小写不敏感路由
- **前置**：只绑定小写 `mine.start`。
- **输入**：`MINE.START`、`Mine.Start`（经解析到达 dispatch 的 verb）
- **预期**：Connector 侧小写归一后命中 `mine.start` handler，调用发生，返回 `Ok`。
- **验证重点**：路由层大小写不敏感的保证（与 CLI11 句法层区分见 `CP-S-016`）。

### CP-L-005 环境作用域：不在矿洞房间时 `mine`
- **前置**：玩家不在矿洞房间（环境中无采矿许可）。
- **输入**：`mine.start --layer 1`
- **预期**：环境校验拦截，提示“需要先进入矿洞”；`MiningHandler.start` **不被调用**、状态不变。
- **验证重点**：作用域校验先于 controller 调用；错误路径后端不执行。

### CP-L-006 目标资源不存在
- **前置**：目标层映射到的矿石不在 `ore.json`。
- **输入**：`mine.start --layer <指向不存在资源的层>`、目标形态 `mine diamond`
- **预期**：资源存在性校验拦截，提示“目标资源不存在”；不进入产量计算、无产出 `MiningResult`。
- **验证重点**：数据缺失时安全短路，不留空产出。

### CP-L-007 方向不可达
- **前置**：地图 `north` 出口不存在 / 不连通。
- **输入**：`go north`
- **预期**：移动控制器环境校验拦截，提示“此方向无法通行”；**不**触发移动副作用、不改变玩家位置。
- **验证重点**：方向可达性在派发后、副作用前被拦截。

### CP-L-008 校验顺序：环境优先于后端
- **前置**：同时具备“不在矿洞”和“等级不足”两类失败条件。
- **输入**：`mine.start --layer 4`
- **步骤**：断言拦截顺序稳定（环境作用域错误优先返回）。
- **预期**：稳定按：状态守卫 → 环境作用域 → 资源/等级 校验顺序返回首个失败；后端 handler 不被调用。
- **验证重点**：多层校验的有序短路，避免后置校验意外触发副作用。

### CP-L-009 bind 覆盖与 has 查询
- **前置**：已绑定 `mine.start`，重复绑定同一 verb。
- **输入**：`bind("mine.start", newHandler)` 后再 `dispatch("mine.start")`
- **预期**：新 handler 覆盖旧 handler，dispatch 命中新实现；`has("mine.start")==true`、`has("north")==false`。
- **验证重点**：注册表覆盖语义与查表接口一致性。

### CP-L-010 后端调用实参校验（副作用证据）
- **前置**：探针 handler 记录每次调用实参。
- **输入**：合法 `mine.start --layer 2` 在空闲且环境/资源通过时。
- **步骤**：断言探针收到的 `HandlerContext.time/mining` 与 `Command`。
- **预期**：探针收到正确 `Command`（verb=`mine.start`、layer=2）且 HandlerContext 引用有效；调用次数恰为 1。
- **验证重点**：跨层引用传递正确、无重复/漏调，为状态结算提供可复核证据。