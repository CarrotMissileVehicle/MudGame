//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_PLAYERSTATECODE_H
#define MUDGAME_PLAYERSTATECODE_H

// DEF-371：无作用域枚举会向每个包含它的翻译单元泄漏 Waiting/Moving 等
// 极常见标识符，易与未来同名枚举/变量冲突；项目其余模块已用 scoped enum
// （enum class MiningStatus / ToolId / WeatherType），这里统一改为 scoped。
enum class StateCode : int {
    Waiting = 10000,
    Moving,

    Watering,
    Seeding,
    Fertilizing,

    Sleeping,

    Shopping,
    Repairing,

    Fishing,

    Mining
};

#endif //MUDGAME_PLAYERSTATECODE_H
