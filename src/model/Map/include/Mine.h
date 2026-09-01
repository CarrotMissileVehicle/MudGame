//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_MINE_H
#define MUDGAME_MINE_H
#include "Position.h"


class Mine : public Position {
    public:
    Mine() : Position(PositionCode::AtMine) {
    }
    Position *GoUp() override;
};


#endif //MUDGAME_MINE_H