//
// Created by opencode on 2026/9/1.
//

#include "Controller/Game/include/Game.h"

using namespace std::chrono;

namespace {

    // 将某时间点（自 epoch 起）转换为游戏日历时间（GameDateTime）
    mud::time::GameDateTime ToTimePoint(system_clock::time_point tp) {
        // 负值（1970 前）视为纪元起点，避免下溢
        const auto floor_days_tp = floor<days>(tp);
        const auto ymd = year_month_day{floor_days_tp};
        const auto time_tod = duration_cast<minutes>(tp - floor_days_tp);
        const auto hhmm = hh_mm_ss(time_tod);
        mud::time::GameDateTime t;
        t.year = static_cast<int>(ymd.year());
        t.month = static_cast<unsigned>(ymd.month());
        t.day = static_cast<unsigned>(ymd.day());
        t.hour = static_cast<unsigned>(hhmm.hours().count());
        t.minute = static_cast<unsigned>(hhmm.minutes().count());
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

mud::time::GameDateTime Game::getSaveOpenTime() const {
    return ToTimePoint(saveOpenTime);
}

mud::time::GameDateTime Game::getSessionStartTime() const {
    return ToTimePoint(sessionStartTime);
}

std::chrono::seconds Game::getTotalPlayTime() const {
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
