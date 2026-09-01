//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_TOWN_H
#define MUDGAME_TOWN_H
#include "Position.h"


class Town :public Position {
public:
    Town() : Position(AtTown) {
    }
    Position *GoDown() override;
};


#endif //MUDGAME_TOWN_H
