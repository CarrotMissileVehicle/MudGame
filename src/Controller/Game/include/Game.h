//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_GAME_H
#define MUDGAME_GAME_H

#include <chrono>
#include "game_time.h"

class Game {
public:
    Game();

    void startSession();
    void endSession();

    [[nodiscard]] mud::time::GameDateTime getSaveOpenTime() const;
    [[nodiscard]] mud::time::GameDateTime getSessionStartTime() const;
    [[nodiscard]] std::chrono::seconds getTotalPlayTime() const;

    // 持久化支持：从存档恢复保存开启时间与总游玩时长
    void setSaveOpenTime(std::chrono::system_clock::time_point time);
    void setTotalPlayTime(std::chrono::seconds total);

private:
    std::chrono::system_clock::time_point saveOpenTime;     // 墙钟时间戳（持久化）
    std::chrono::system_clock::time_point sessionStartWall; // 会话开始墙钟时间（展示用）
    std::chrono::steady_clock::time_point sessionStartTime; // 单调时钟（测量时长，防时钟回拨）
    std::chrono::milliseconds totalPlayTime;                // 毫秒累计，避免秒级截断丢失
    bool sessionActive;
};

#endif //MUDGAME_GAME_H
