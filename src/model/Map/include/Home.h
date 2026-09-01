//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_HOME_H
#define MUDGAME_HOME_H
#include "Position.h"


class Home : public Position {
public:
    explicit Home() : Position(AtHome) {
    }
    Position *GoUp() override;
    Position *GoDown() override;
    Position *GoLeft() override;
    Position *GoRight() override;
};


#endif //MUDGAME_HOME_H
