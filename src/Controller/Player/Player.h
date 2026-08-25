//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_PLAYER_H
#define MUDGAME_PLAYER_H
#include "../Bag/include/Bag.h"
#include "../Map/include/Position.h"
#include "../PlayerStates/include/PlayerState.h"

class Player {
public:
    Player();

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
