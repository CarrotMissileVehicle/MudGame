/**
 * @file GameCommands.h
 * @brief 命令处理器注册表：把全部指令绑定到 Connector（含参数 Schema）。
 *
 * 按领域拆分为多个实现文件（commands_mining / commands_farm / commands_market /
 * commands_blacksmith / commands_misc），每文件均保持在 200 行以内。
 * 所有处理器通过 GameContext 访问共享世界状态。
 */
#pragma once

class Connector;
class GameContext;
class WorldEngine;

/**
 * @brief 命令注册表：集中式注册全部指令（迁移目标）。
 *
 * @note 当前状态（2026-09）：本门面尚无调用点——src/Controller/GameSession/
 *       尚未编入 MudGame 可执行目标，main.cpp 仍以内联方式逐一 bind 指令并
 *       重复注册 Schema（main.cpp 约 509-1086 行）。两处动词/Schema 表互为
 *       镜像，修改时必须同步；完成 GameSession 装配迁移后应删除 main.cpp
 *       的内联注册、改调 register_all，恢复单一事实来源。
 */
class GameCommands
{
public:
    /// 注册全部命令处理器与参数 Schema。
    static void register_all(Connector& connector, GameContext& ctx, WorldEngine& world);

    // ---- 分域注册 ----
    static void register_mining(Connector& connector, GameContext& ctx, WorldEngine& world);
    static void register_farm(Connector& connector, GameContext& ctx);
    static void register_market(Connector& connector, GameContext& ctx);
    static void register_blacksmith(Connector& connector, GameContext& ctx);
    static void register_misc(Connector& connector, GameContext& ctx, WorldEngine& world);

    /// 命令参数 Schema 注册（交互模式逐参数提示）。
    static void register_schemas(Connector& connector, GameContext& ctx);
};