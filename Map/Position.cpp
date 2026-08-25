//
// Created by z2996 on 2026/8/25.
//

#include "Position.h"

Position::Position(PositionCode pos) {
    this->pos = pos;
}

PositionCode Position::GetCode() const {
    return pos;
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
