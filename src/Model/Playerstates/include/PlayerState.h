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

    // DEF-375：仅接受 StateCode 构造。int 重载曾让任意整数静默注入
    // 非法状态码（无作用域枚举可隐式转 int），移除后编译期即拦截。
    explicit PlayerState(StateCode code);

    [[nodiscard]] const std::vector<StateCode> *GetAbleStatesByPos(PositionCode pos) const;

    [[nodiscard]] StateCode GetState() const;

    void SetState(StateCode state);

private:
    StateCode stateCode;
    std::map<PositionCode, std::vector<StateCode> > ableState = {
        {AtHome, {StateCode::Sleeping}},
        {AtTown, {StateCode::Shopping, StateCode::Repairing}},
        {AtCoast, {StateCode::Fishing}},
        {AtMine, {StateCode::Mining}},
        {AtFarmland, {StateCode::Seeding, StateCode::Watering, StateCode::Fertilizing}}
    };
};


#endif //MUDGAME_PLAYERSTATE_H
