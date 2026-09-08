//
// Created by opencode on 2026/9/1.
//

#include "Controller/Player/include/Move.h"

#include <iostream>
#include <memory>

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

bool Move::GoUp() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    return MoveTo(pos->GoUp());
}

bool Move::GoDown() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    return MoveTo(pos->GoDown());
}

bool Move::GoLeft() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    return MoveTo(pos->GoLeft());
}

bool Move::GoRight() {
    const auto pos = concrete_position(player.GetPosition());
    if (pos == nullptr) return false;
    return MoveTo(pos->GoRight());
}

bool Move::MoveTo(Position* newPos) {
    if (newPos == nullptr) {
        return false;
    }
    player.SetPosition(newPos->GetCode());
    player.SetState(StateCode::Waiting);
    delete newPos;
    return true;
}
