//
// Created by opencode on 2026/9/1.
//

#include "controller/Game/include/Game.h"

using namespace std::chrono;

Game::Game()
    : saveOpenTime(system_clock::now()),
      sessionStartTime(system_clock::now()),
      totalPlayTime(0),
      sessionActive(false) {
}

void Game::startSession() {
    if (sessionActive) {
        return;
    }
    sessionStartTime = system_clock::now();
    sessionActive = true;
}

void Game::endSession() {
    if (!sessionActive) {
        return;
    }
    auto elapsed = duration_cast<seconds>(system_clock::now() - sessionStartTime);
    totalPlayTime += elapsed;
    sessionActive = false;
}

system_clock::time_point Game::getSaveOpenTime() const {
    return saveOpenTime;
}

system_clock::time_point Game::getSessionStartTime() const {
    return sessionStartTime;
}

seconds Game::getTotalPlayTime() const {
    auto total = totalPlayTime;
    if (sessionActive) {
        total += duration_cast<seconds>(system_clock::now() - sessionStartTime);
    }
    return total;
}

void Game::setSaveOpenTime(system_clock::time_point time) {
    saveOpenTime = time;
}

void Game::setTotalPlayTime(seconds total) {
    totalPlayTime = total;
}
