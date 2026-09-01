//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_FARMLAND_H
#define MUDGAME_FARMLAND_H
#include "Position.h"


class Farmland : Position {
public:
    Farmland() : Position(PositionCode::Farmland) {
    }
    Position *GoRight() override;
};


#endif //MUDGAME_FARMLAND_H
