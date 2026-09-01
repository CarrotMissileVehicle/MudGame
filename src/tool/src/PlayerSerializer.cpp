//
// Created by opencode on 2026/9/1.
//

#include "../include/PlayerSerializer.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool PlayerSerializer::Save(const std::string& filename, const Player& player, const Game& game) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件进行写入: " << filename << std::endl;
        return false;
    }

    file << "# 玩家存档文件\n";
    file << "position=" << GetPositionName(player.GetPosition()) << "\n";
    file << "state=" << GetStateName(player.GetState()) << "\n";
    file << "satiety=" << player.GetSatiety() << "\n";
    file << "maxSatiety=" << player.GetMaxSatiety() << "\n";
    file << "farmingExp=" << player.GetFarmingExp() << "\n";
    file << "fishExp=" << player.GetFishExp() << "\n";
    file << "mineExp=" << player.GetMineExp() << "\n";

    auto saveOpenEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
            game.getSaveOpenTime().time_since_epoch()).count();
    file << "saveOpenTime=" << saveOpenEpoch << "\n";
    file << "totalPlayTime=" << game.getTotalPlayTime().count() << "\n";

    const auto& bag = player.GetBag();
    auto itemNames = bag.GetAllObjectName();
    file << "bagCount=" << itemNames.size() << "\n";

    const auto& objects = bag.GetObjects();
    for (const auto* obj : objects) {
        file << "item=" << obj->GetName() << "|"
             << obj->GetDescription() << "|"
             << obj->GetHealth() << "|"
             << obj->GetSellingPrice() << "|"
             << obj->GetBuyingPrice() << "\n";
    }

    file.close();
    std::cout << "存档成功: " << filename << std::endl;
    return true;
}

bool PlayerSerializer::Save(const std::string& filename, const Player& player) {
    Game game;
    return Save(filename, player, game);
}

bool PlayerSerializer::Load(const std::string& filename, Player& player, Game& game) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开存档文件: " << filename << std::endl;
        return false;
    }

    PositionCode pos = AtHome;
    StateCode state = Waiting;
    int satiety = 100, maxSatiety = 100;
    int farmingExp = 0, fishExp = 0, mineExp = 0;

    long long saveOpenEpoch = 0;
    long long totalPlaySeconds = 0;

    std::string line;
    int bagCount = 0;
    std::vector<std::string> itemLines;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "position") {
                pos = ParsePosition(value);
            } else if (key == "state") {
                state = ParseState(value);
            } else if (key == "satiety") {
                satiety = std::stoi(value);
            } else if (key == "maxSatiety") {
                maxSatiety = std::stoi(value);
            } else if (key == "farmingExp") {
                farmingExp = std::stoi(value);
            } else if (key == "fishExp") {
                fishExp = std::stoi(value);
            } else if (key == "mineExp") {
                mineExp = std::stoi(value);
            } else if (key == "saveOpenTime") {
                saveOpenEpoch = std::stoll(value);
            } else if (key == "totalPlayTime") {
                totalPlaySeconds = std::stoll(value);
            } else if (key == "bagCount") {
                bagCount = std::stoi(value);
            } else if (key == "item") {
                itemLines.push_back(value);
            }
        }
    }

    player = Player(pos, state, satiety, maxSatiety, farmingExp, fishExp, mineExp);

    if (saveOpenEpoch > 0) {
        game.setSaveOpenTime(std::chrono::system_clock::time_point(
                std::chrono::milliseconds(saveOpenEpoch)));
    }
    game.setTotalPlayTime(std::chrono::seconds(totalPlaySeconds));

    for (const auto& itemLine : itemLines) {
        std::istringstream itemIss(itemLine);
        std::string name, description, healthStr, sellStr, buyStr;
        if (std::getline(itemIss, name, '|') &&
            std::getline(itemIss, description, '|') &&
            std::getline(itemIss, healthStr, '|') &&
            std::getline(itemIss, sellStr, '|') &&
            std::getline(itemIss, buyStr)) {
            int health = std::stoi(healthStr);
            int sellPrice = std::stoi(sellStr);
            int buyPrice = std::stoi(buyStr);
            Object* obj = new Object(name, description, health, sellPrice, buyPrice);
            player.GetBag().AddObject(obj);
        }
    }

    file.close();
    std::cout << "读档成功: " << filename << std::endl;
    return true;
}

bool PlayerSerializer::Load(const std::string& filename, Player& player) {
    Game game;
    return Load(filename, player, game);
}

std::string PlayerSerializer::GetPositionName(PositionCode code) const {
    switch (code) {
        case AtHome:      return "Home";
        case AtFarmland:  return "Farmland";
        case AtCoast:     return "Coast";
        case AtMine:      return "Mine";
        case AtTown:      return "Town";
        default:        return "Unknown";
    }
}

std::string PlayerSerializer::GetStateName(StateCode code) const {
    switch (code) {
        case Waiting:       return "Waiting";
        case Moving:        return "Moving";
        case Watering:      return "Watering";
        case Seeding:       return "Seeding";
        case Fertilizing:   return "Fertilizing";
        case Sleeping:      return "Sleeping";
        case Shopping:      return "Shopping";
        case Repairing:     return "Repairing";
        case Fishing:       return "Fishing";
        case Mining:        return "Mining";
        default:            return "Unknown";
    }
}

PositionCode PlayerSerializer::ParsePosition(const std::string& name) const {
    if (name == "Home") return AtHome;
    if (name == "Farmland") return AtFarmland;
    if (name == "Coast") return AtCoast;
    if (name == "Mine") return AtMine;
    if (name == "Town") return AtTown;
    return AtHome;
}

StateCode PlayerSerializer::ParseState(const std::string& name) const {
    if (name == "Waiting") return Waiting;
    if (name == "Moving") return Moving;
    if (name == "Watering") return Watering;
    if (name == "Seeding") return Seeding;
    if (name == "Fertilizing") return Fertilizing;
    if (name == "Sleeping") return Sleeping;
    if (name == "Shopping") return Shopping;
    if (name == "Repairing") return Repairing;
    if (name == "Fishing") return Fishing;
    if (name == "Mining") return Mining;
    return Waiting;
}
