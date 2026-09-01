//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_COAST_H
#define MUDGAME_COAST_H
#include "Position.h"


class Coast :public Position {
public:
    Coast() : Position(PositionCode::AtCoast) {
    }

    Position *GoLeft() override;
};


#endif //MUDGAME_COAST_H
