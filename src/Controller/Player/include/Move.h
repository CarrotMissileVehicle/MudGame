//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_MOVE_H
#define MUDGAME_MOVE_H

#include "Player.h"

class Move {
public:
    explicit Move(Player& player);

    bool GoUp();
    bool GoDown();
    bool GoLeft();
    bool GoRight();

private:
    Player& player;
    bool MoveTo(Position* newPos);
};

#endif //MUDGAME_MOVE_H
