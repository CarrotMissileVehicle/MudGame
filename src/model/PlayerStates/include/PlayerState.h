//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_PLAYERSTATE_H
#define MUDGAME_PLAYERSTATE_H
#include <map>
#include <vector>
#include "../../Map/include/PositionCode.h"
#include "PlayerStateCode.h"

class PlayerState {
public:
    virtual ~PlayerState() = default;

    explicit PlayerState(int code);
    explicit PlayerState(StateCode code);

    [[nodiscard]] const std::vector<StateCode> *GetAbleStatesByPos(PositionCode pos) const;

    [[nodiscard]] StateCode GetState() const;

    void SetState(StateCode state);

private:
    StateCode stateCode;
    std::map<PositionCode, std::vector<StateCode> > ableState = {
        {AtHome, {Sleeping}},
        {AtTown, {Shopping, Repairing}},
        {AtCoast, {Fishing}},
        {AtMine, {Mining}},
        {AtFarmland, {Seeding, Watering, Fertilizing}}
    };
};


#endif //MUDGAME_PLAYERSTATE_H
