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

    // 具体位置（Home/Town/Farmland/Mine/Coast）按移动图覆盖对应方向。
    // 基类默认实现抛 std::logic_error（H13）：调用方须保证所在位置覆盖了
    // 该方向，或捕获异常（Move 层已处理为“走不通”），不得静默依赖空指针。
    [[nodiscard]] virtual Position *GoUp();
    [[nodiscard]] virtual Position *GoDown();
    [[nodiscard]] virtual Position *GoLeft();
    [[nodiscard]] virtual Position *GoRight();

private:
    PositionCode pos;
};


#endif //MUDGAME_POSITION_H
