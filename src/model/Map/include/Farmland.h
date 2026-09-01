//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_FARMLAND_H
#define MUDGAME_FARMLAND_H
#include "Position.h"


class Farmland : public Position {
public:
    Farmland() : Position(PositionCode::AtFarmland) {
    }
    Position *GoRight() override;
};


#endif //MUDGAME_FARMLAND_H
