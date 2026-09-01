//
// Created by z2996 on 2026/8/25.
//

#include "../include/Player.h"

Player::Player()
    : position(AtHome), state(Waiting), satiety(100), maxSatiety(100),
      farmingExperience(0), fishExperience(0), mineExperience(0) {
}

Player::Player(PositionCode pos, StateCode stateCode, int satiety, int maxSatiety,
               int farmingExp, int fishExp, int mineExp)
    : position(pos), state(stateCode), satiety(satiety), maxSatiety(maxSatiety),
      farmingExperience(farmingExp), fishExperience(fishExp), mineExperience(mineExp) {
}

PositionCode Player::GetPosition() const {
    return position.GetCode();
}

void Player::SetPosition(PositionCode pos) {
    position.SetCode(pos);
}

StateCode Player::GetState() const {
    return state.GetState();
}

void Player::SetState(StateCode newState) {
    state.SetState(newState);
}

int Player::GetSatiety() const {
    return satiety;
}

int Player::GetMaxSatiety() const {
    return maxSatiety;
}

void Player::SetSatiety(int value) {
    satiety = value;
}

void Player::SetMaxSatiety(int value) {
    maxSatiety = value;
}

int Player::GetFarmingExp() const {
    return farmingExperience;
}

int Player::GetFishExp() const {
    return fishExperience;
}

int Player::GetMineExp() const {
    return mineExperience;
}

void Player::SetFarmingExp(int value) {
    farmingExperience = value;
}

void Player::SetFishExp(int value) {
    fishExperience = value;
}

void Player::SetMineExp(int value) {
    mineExperience = value;
}

Bag& Player::GetBag() {
    return bag;
}

const Bag& Player::GetBag() const {
    return bag;
}