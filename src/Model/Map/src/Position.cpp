//
// Created by z2996 on 2026/8/25.
//

#include "../include/Position.h"

#include <stdexcept>

Position::Position(PositionCode pos) {
    this->pos = pos;
}


PositionCode Position::GetCode() const {
    return pos;
}

void Position::SetCode(PositionCode code) {
    this->pos = code;
}

// H13：基类不实现具体移动图。未覆盖的方向此前静默返回空指针，
// 任何 "pos->GoUp()->GetCode()" 式调用都会解引用崩溃；改为显式抛错，
// 由 Move 层捕获并降级为“走不通”，失败不再静默。
Position *Position::GoUp() {
    throw std::logic_error("未实现的导航方向: GoUp");
}

Position *Position::GoDown() {
    throw std::logic_error("未实现的导航方向: GoDown");
}

Position *Position::GoLeft() {
    throw std::logic_error("未实现的导航方向: GoLeft");
}

Position *Position::GoRight() {
    throw std::logic_error("未实现的导航方向: GoRight");
}
