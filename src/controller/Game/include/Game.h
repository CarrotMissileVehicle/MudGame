//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_GAME_H
#define MUDGAME_GAME_H

#include <chrono>

class Game {
public:
    Game();

    void startSession();
    void endSession();

    [[nodiscard]] std::chrono::system_clock::time_point getSaveOpenTime() const;
    [[nodiscard]] std::chrono::system_clock::time_point getSessionStartTime() const;
    [[nodiscard]] std::chrono::seconds getTotalPlayTime() const;

    // 持久化支持：从存档恢复保存开启时间与总游玩时长
    void setSaveOpenTime(std::chrono::system_clock::time_point time);
    void setTotalPlayTime(std::chrono::seconds total);

private:
    std::chrono::system_clock::time_point saveOpenTime;
    std::chrono::system_clock::time_point sessionStartTime;
    std::chrono::seconds totalPlayTime;
    bool sessionActive;
};

#endif //MUDGAME_GAME_H
