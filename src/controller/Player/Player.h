//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_PLAYER_H
#define MUDGAME_PLAYER_H
#include "../../model/Bag/include/Bag.h"
#include "../../model/Map/include/Position.h"
#include "../../model/PlayerStates/include/PlayerState.h"

class Player {
public:
    Player();
    Player(PositionCode pos, StateCode state, int satiety, int maxSatiety,
           int farmingExp, int fishExp, int mineExp);

    // Position
    [[nodiscard]] PositionCode GetPosition() const;
    void SetPosition(PositionCode pos);

    // State
    [[nodiscard]] StateCode GetState() const;
    void SetState(StateCode state);

    // Satiety
    [[nodiscard]] int GetSatiety() const;
    [[nodiscard]] int GetMaxSatiety() const;
    void SetSatiety(int value);
    void SetMaxSatiety(int value);

    // Experience
    [[nodiscard]] int GetFarmingExp() const;
    [[nodiscard]] int GetFishExp() const;
    [[nodiscard]] int GetMineExp() const;
    void SetFarmingExp(int value);
    void SetFishExp(int value);
    void SetMineExp(int value);

    // Bag
    Bag& GetBag();
    [[nodiscard]] const Bag& GetBag() const;

private:
    Position position;
    PlayerState state;
    Bag bag;
    int satiety;
    int maxSatiety;
    int farmingExperience;
    int fishExperience;
    int mineExperience;
};


#endif //MUDGAME_PLAYER_H
