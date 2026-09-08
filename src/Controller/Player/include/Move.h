//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_MOVE_H
#define MUDGAME_MOVE_H

#include <memory>

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
    // DEF-011：unique_ptr 接管 GoX() 返回的 new 对象，编译期强制所有权，
    // 杜绝「调用方漏 delete 即泄漏」的隐式约定。
    bool MoveTo(std::unique_ptr<Position> newPos);
};

#endif //MUDGAME_MOVE_H
