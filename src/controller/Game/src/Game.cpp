//
// Created by opencode on 2026/9/1.
//

#include "controller/Game/include/Game.h"

using namespace std::chrono;

namespace {

    // 将某时间点（自 epoch 起）转换为 Time（day: 天数，hour: 0-23，minute: 0-59）
    Time ToTimePoint(system_clock::time_point tp) {
        auto secs = duration_cast<seconds>(tp.time_since_epoch()).count();
        Time t;
        t.day = static_cast<int>(secs / 86400);
        t.hour = static_cast<int>((secs / 3600) % 24);
        t.minute = static_cast<int>((secs / 60) % 60);
        return t;
    }

    // 将时长（秒）转换为 Time（day: 总天数，hour: 余下小时，minute: 余下分钟）
    Time ToTimeDuration(seconds s) {
        auto total = s.count();
        Time t;
        t.day = static_cast<int>(total / 86400);
        t.hour = static_cast<int>((total / 3600) % 24);
        t.minute = static_cast<int>((total / 60) % 60);
        return t;
    }

} // namespace

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

Time Game::getSaveOpenTime() const {
    return ToTimePoint(saveOpenTime);
}

Time Game::getSessionStartTime() const {
    return ToTimePoint(sessionStartTime);
}

Time Game::getTotalPlayTime() const {
    auto total = totalPlayTime;
    if (sessionActive) {
        total += duration_cast<seconds>(system_clock::now() - sessionStartTime);
    }
    return ToTimeDuration(total);
}

void Game::setSaveOpenTime(system_clock::time_point time) {
    saveOpenTime = time;
}

void Game::setTotalPlayTime(seconds total) {
    totalPlayTime = total;
}
