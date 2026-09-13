//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_PLAYERSERIALIZER_H
#define MUDGAME_PLAYERSERIALIZER_H

#include <cstdint>
#include <string>
#include "../../Controller/Player/include/Player.h"
#include "../../Controller/Game/include/Game.h"
#include "../../Controller/Tool/include/tool_controller.h"

class PlayerSerializer {
public:
    // 完整存档：玩家 + 游戏会话 + 金币 + 工具状态 + 游戏内时钟总分钟数
    // 金币为 64 位（DEF-007：与 Market/main 的 long long gold 对齐）
    bool Save(const std::string& filename, const Player& player, const Game& game, long long gold,
              const mud::tool::ToolController& tools, std::int64_t totalGameMinutes);
    bool Load(const std::string& filename, Player& player, Game& game, long long& gold,
              mud::tool::ToolController& tools, std::int64_t& totalGameMinutes);

    bool Save(const std::string& filename, const Player& player);
    bool Load(const std::string& filename, Player& player);

private:
    std::string GetPositionName(PositionCode code) const;
    std::string GetStateName(StateCode code) const;
    PositionCode ParsePosition(const std::string& name) const;
    StateCode ParseState(const std::string& name) const;
};

#endif //MUDGAME_PLAYERSERIALIZER_H
