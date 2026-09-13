# 修复报告（RepairReport.md）

> 目标：参考 `docs/repair.md` 设计 TUI 界面，每个阶段完成后在本文件末尾追加报告。

## 阶段一：拉取 GitHub 上最受欢迎的 TUI 库 — FTXUI

**选型结论**：选用 **FTXUI**（`Arthapz/Feline` 与 `ArthurSonzogni/FTXUI` 中 star 数最高、维护最活跃的 C++ TUI 库，10k+ stars），版本 **v7.0.3**。

**网络说明**：本机无法直连 `github.com`（TCP 443 握手失败）。已确认 Gitee 可达，因此通过 **Gitee 镜像** `https://gitee.com/mirrors/ftxui.git` 拉取，二者上游同一；`git ls-remote` 验证 tag `v7.0.3`（commit `f921fad208912747c17d129a8ef75ec7624b6eec`）存在。

**集成方式**（根 `CMakeLists.txt`）：

```cmake
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://gitee.com/mirrors/ftxui.git
    GIT_TAG v7.0.3
    GIT_SHALLOW TRUE
)
set(FTXUI_BUILD_EXAMPLES OFF)
set(FTXUI_BUILD_DOCS OFF)
set(FTXUI_BUILD_TESTS OFF)
FetchContent_MakeAvailable(ftxui)
```

MudGame 可执行目标链接 `ftxui::component`、`ftxui::dom`、`ftxui::screen`。

**验证**：MinGW（CLion 内置 g++）下约 80 个目标全部编译成功，无错误。

---

## 阶段二：TUI 布局设计（新建 `src/View/Tui` 模块）

按目标书要求实现四块区域，自上而下：

```
┌────────────────────────────────────────────┐
│ 日志区（可滚动、占满全部高度）              │  ← 左上方日志
├────────────────────────────────────────────┤
│ [候选项列表]（有输入时按前缀动态列出）      │  ← 动态提示列表
│  问题行（如：请输入 crop：作物名(...)）     │  ← 问题上显示
│ > 输入命令（help 查看帮助）               │  ← 下方输入指令
└────────────────────────────────────────────┘
```

| 文件 | 职责 |
|---|---|
| `src/View/Tui/include/TuiState.h` | 界面共享状态：日志队列（上限 2000 行）、问题行、候选项、输入框内容、非阻塞动作状态 |
| `src/View/Tui/include/TuiRenderer.h` | `Renderer` 实现，将 View 层所有输出重定向到日志区 |
| `src/View/Tui/include/GameTui.h` | 宿主 API：`set_process_line` / `set_completions` / `set_tick` / `run` / `exit` / `post_background` |
| `src/View/Tui/src/GameTui.cpp` | FTXUI v7 组件树与事件循环（`App::Fullscreen` + `Loop`） |

**动态提示列表规则**（严格按目标书）：

- 空闲且有输入 → 按输入前缀过滤已注册动词（如输入 `farm.` 列出 `farm.*`）。
- **无输入 → 列表隐藏**。
- 参数收集中 → 按输入过滤当前参数的候选值（作物名 / 地块号 / 层号 / 工具名等）。
- 支持 **Tab** 直接采用首个候选项填入输入框。

**交互约定**：`Enter` 执行命令；`Ctrl+C` 或 `quit` 退出；`q` 取消参数输入或结束进行中的动作。

---

## 阶段三：接入游戏逻辑（main.cpp 重构 REPL）

目标书的核心诉求是把原先阻塞式 REPL 替换为 TUI 主界面，需解决三处阻塞：

1. **输出通道**：以 `TuiRenderer` 替换 `StdoutRenderer`，`TerminalView` 面板输出全部进入日志区；idle 时上方显示当前位置指令提示。
2. **参数收集**：原 `ParameterCollector`（`getline` 阻塞）改为 **TUI 状态机**：输入动词 → 若注册了 schema 则进入逐参数收集，`question` 行提示当前参数、`completions` 随输入过滤；全部填毕后统一 `dispatch`。
3. **非阻塞动作**：
   - 钓鱼：`fish.tick` 启动后由 1 秒后台节拍的 `handle_action_tick` 按 3-6 秒随机间隔结算（耗体力 → `tickFish` → 入包/加经验），不再 `_getch` 轮询。
   - 采矿：`mine.start` 改为 `MiningHandler::start` 建立会话，产出交由既有后台 `tick_world` 的 `poll` 路径按游戏分钟结算；`q` / `mine.stop` 结束。

**线程模型**：保留 `worldMutex`；后台 `jthread` 每现实秒经 `GameTui::post_background`（FTXUI 线程安全的 `App::PostEventOrExecute`）在主线程执行 `world_step` = `tick_world` + 动作推进 + 提示刷新。

---

## 阶段四：构建与测试验证

- **编译**：全量 `cmake --build` 成功，0 错误、0 警告（MinGW / C++20）。
- **单元测试**：`ctest --output-on-failure` **28/28 全部通过**（cmdparser、timeservice、mining、tool、serializer、view、weather 等）。
- **冒烟测试**：`MudGame.exe` 启动后运行 4 秒无崩溃、无 stderr 输出；退出正常清理后台线程并 `endSession`。
```