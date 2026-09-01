//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_POSITION_H
#define MUDGAME_POSITION_H
#include "PositionCode.h"


class Position {
public:
    virtual ~Position() = default;

    explicit Position(PositionCode pos);

    [[nodiscard]] PositionCode GetCode() const;
    void SetCode(PositionCode code);

    virtual Position *GoUp();
    virtual Position *GoDown();
    virtual Position *GoLeft();
    virtual Position *GoRight();

private:
    PositionCode pos;
};


#endif //MUDGAME_POSITION_H
