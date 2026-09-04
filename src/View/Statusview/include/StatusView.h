//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_STATUSVIEW_H
#define MUDGAME_STATUSVIEW_H

#include <string>
#include <vector>

#include "../../../Model/Map/include/PositionCode.h"
#include "../../../Model/Playerstates/include/PlayerStateCode.h"

struct PlayerStatus {
    PositionCode position;
    StateCode state;
    int satiety;
    int maxSatiety;
    int farmingExp;
    int fishExp;
    int mineExp;
    std::vector<std::string> bagItems;
};

class StatusView {
public:
    void ShowStatus(const PlayerStatus& status);

private:
    std::string GetPositionName(PositionCode code) const;
    std::string GetStateName(StateCode code) const;
};

#endif //MUDGAME_STATUSVIEW_H
