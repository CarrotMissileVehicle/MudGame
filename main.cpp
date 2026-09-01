#include <iostream>
#include <filesystem>

#include "controller/Game/include/Game.h"
#include "controller/Player/include/Player.h"
#include "tool/include/PlayerSerializer.h"

namespace {
    const std::string SAVE_FILE = "save.txt";
}

int main() {
    Player player;
    Game game;
    PlayerSerializer serializer;

    // 读档：若存在存档，恢复存档开启时间与总游玩时长；否则为新档，记录当前为存档开启时间
    bool isNewSave = !std::filesystem::exists(SAVE_FILE);
    if (!isNewSave) {
        if (serializer.Load(SAVE_FILE, player, game)) {
            std::cout << "读取存档: " << SAVE_FILE << std::endl;
        } else {
            std::cout << "存档读取失败，将开始新游戏。" << std::endl;
        }
    } else {
        std::cout << "新游戏开始，存档开启时间: " << std::endl;
    }

    // 游戏开始时自动记录会话开启时间
    game.startSession();

    std::cout << "存档开启时间: "
              << game.getSaveOpenTime().day << "天 "
              << game.getSaveOpenTime().hour << "时 "
              << game.getSaveOpenTime().minute << "分" << std::endl;
    std::cout << "会话开启时间: "
              << game.getSessionStartTime().day << "天 "
              << game.getSessionStartTime().hour << "时 "
              << game.getSessionStartTime().minute << "分" << std::endl;
    std::cout << "游戏总时长: "
              << game.getTotalPlayTime().day << "天 "
              << game.getTotalPlayTime().hour << "时 "
              << game.getTotalPlayTime().minute << "分" << std::endl;

    // 模拟游玩
    std::cout << "正在游玩... (输入任意字符后回车退出)" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    // 游戏退出时自动记录游玩时长，并持久化存档
    game.endSession();
    if (serializer.Save(SAVE_FILE, player, game)) {
        std::cout << "存档成功，本次新累计游戏时长: "
                  << game.getTotalPlayTime().day << "天 "
                  << game.getTotalPlayTime().hour << "时 "
                  << game.getTotalPlayTime().minute << "分" << std::endl;
    }

    return 0;
}
