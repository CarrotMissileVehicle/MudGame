//
// Created by z2996 on 2026/8/25.
//

#include "../include/Position.h"

Position::Position(PositionCode pos) {
    this->pos = pos;
}


PositionCode Position::GetCode() const {
    return pos;
}

void Position::SetCode(PositionCode code) {
    this->pos = code;
}

Position *Position::GoUp() {
    return nullptr;
}

Position *Position::GoDown() {
    return nullptr;
}

Position *Position::GoLeft() {
    return nullptr;
}

Position *Position::GoRight() {
    return nullptr;
}
