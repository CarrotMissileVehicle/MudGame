//
// Created by opencode on 2026/9/1.
//

#include "Controller/Player/include/Move.h"

#include <iostream>
#include <memory>
#include <stdexcept>

#include "Model/Map/include/Coast.h"
#include "Model/Map/include/Farmland.h"
#include "Model/Map/include/Home.h"
#include "Model/Map/include/Mine.h"
#include "Model/Map/include/Town.h"

namespace
{
    // 按当前所在地实例化具体位置对象，使 GoX 虚拟分派到对应移动图。
    std::unique_ptr<Position> concrete_position(PositionCode code)
    {
        switch (code)
        {
        case AtHome:     return std::make_unique<Home>();
        case AtTown:     return std::make_unique<Town>();
        case AtFarmland: return std::make_unique<Farmland>();
        case AtMine:     return std::make_unique<Mine>();
        case AtCoast:    return std::make_unique<Coast>();
        default:         return nullptr;
        }
    }
} // namespace

Move::Move(Player& player) : player(player) {
}

// 该位置未覆盖的导航方向由基类抛 std::logic_error（H13），
// 捕获后视为“走不通”返回 false，保持玩家体验不崩溃
bool Move::GoUp() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    try {
        return MoveTo(std::unique_ptr<Position>(pos->GoUp()));
    } catch (const std::logic_error&) {
        return false;
    }
}

bool Move::GoDown() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    try {
        return MoveTo(std::unique_ptr<Position>(pos->GoDown()));
    } catch (const std::logic_error&) {
        return false;
    }
}

bool Move::GoLeft() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    try {
        return MoveTo(std::unique_ptr<Position>(pos->GoLeft()));
    } catch (const std::logic_error&) {
        return false;
    }
}

bool Move::GoRight() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    try {
        return MoveTo(std::unique_ptr<Position>(pos->GoRight()));
    } catch (const std::logic_error&) {
        return false;
    }
}

bool Move::MoveTo(std::unique_ptr<Position> newPos) {
    if (newPos == nullptr) {
        return false;
    }
    player.SetPosition(newPos->GetCode());
    player.SetState(StateCode::Waiting);
    return true;   // newPos 随 unique_ptr 离开作用域自动释放（DEF-011）
}
