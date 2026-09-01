//
// Created by opencode on 2026/9/1.
//

#include "controller/Player/include/Move.h"

#include <iostream>

Move::Move(Player& player) : player(player) {
}

bool Move::GoUp() {
    return MoveTo(Position(player.GetPosition()).GoUp());
}

bool Move::GoDown() {
    return MoveTo(Position(player.GetPosition()).GoDown());
}

bool Move::GoLeft() {
    return MoveTo(Position(player.GetPosition()).GoLeft());
}

bool Move::GoRight() {
    return MoveTo(Position(player.GetPosition()).GoRight());
}

bool Move::MoveTo(Position* newPos) {
    if (newPos == nullptr) {
        return false;
    }
    player.SetPosition(newPos->GetCode());
    player.SetState(StateCode::Waiting);
    delete newPos;
    return true;
}
