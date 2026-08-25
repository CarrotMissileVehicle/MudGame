//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_HOME_H
#define MUDGAME_HOME_H
#include "Position.h"


class Home : Position {
public:
    explicit Home() : Position(PositionCode::Home) {
    }
    Position *GoUp() override;
    Position *GoDown() override;
    Position *GoLeft() override;
    Position *GoRight() override;
};


#endif //MUDGAME_HOME_H
