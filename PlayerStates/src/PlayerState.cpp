//
// Created by z2996 on 2026/8/25.
//

#include "../include/PlayerState.h"

PlayerState::PlayerState(int code) : stateCode(Waiting) {
}

const std::vector<StateCode> *PlayerState::GetAbleStatesByPos(PositionCode pos) const {
    return &ableState.at(pos);
}

StateCode PlayerState::GetState() const {
    return stateCode;
}

void PlayerState::SetState(StateCode state) {
    this->stateCode = state;
}
