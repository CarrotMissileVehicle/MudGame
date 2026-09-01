//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_PLAYERSERIALIZER_H
#define MUDGAME_PLAYERSERIALIZER_H

#include <string>
#include "../../controller/Player/include/Player.h"
#include "../../controller/Game/include/Game.h"

class PlayerSerializer {
public:
    bool Save(const std::string& filename, const Player& player, const Game& game);
    bool Load(const std::string& filename, Player& player, Game& game);

    bool Save(const std::string& filename, const Player& player);
    bool Load(const std::string& filename, Player& player);

private:
    std::string GetPositionName(PositionCode code) const;
    std::string GetStateName(StateCode code) const;
    PositionCode ParsePosition(const std::string& name) const;
    StateCode ParseState(const std::string& name) const;
};

#endif //MUDGAME_PLAYERSERIALIZER_H
