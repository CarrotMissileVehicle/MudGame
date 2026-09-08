//
// Created by opencode on 2026/9/1.
//

#include "../include/PlayerSerializer.h"
#include <algorithm>
#include <fstream>
#include <sstream>

bool PlayerSerializer::Save(const std::string& filename, const Player& player, const Game& game,
                            int gold, const mud::tool::ToolController& tools,
                            std::int64_t totalGameMinutes) {
    std::ofstream file(filename);
    if (!file.is_open()) {
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
    file << "gold=" << gold << "\n";

    mud::time::GameDateTime saveOpen = game.getSaveOpenTime();
    file << "saveOpenYear=" << saveOpen.year << "\n";
    file << "saveOpenMonth=" << saveOpen.month << "\n";
    file << "saveOpenDay=" << saveOpen.day << "\n";
    file << "saveOpenHour=" << saveOpen.hour << "\n";
    file << "saveOpenMinute=" << saveOpen.minute << "\n";

    auto totalPlaySeconds = game.getTotalPlayTime().count();
    file << "totalPlaySeconds=" << totalPlaySeconds << "\n";
    file << "gameTotalMinutes=" << totalGameMinutes << "\n";

    // 工具状态：等级 + 耐久
    file << "toolHoeLevel=" << tools.level(mud::tool::ToolId::Hoe) << "\n";
    file << "toolHoeDurability=" << tools.durability(mud::tool::ToolId::Hoe) << "\n";
    file << "toolRodLevel=" << tools.level(mud::tool::ToolId::Rod) << "\n";
    file << "toolRodDurability=" << tools.durability(mud::tool::ToolId::Rod) << "\n";
    file << "toolPickLevel=" << tools.level(mud::tool::ToolId::Pickaxe) << "\n";
    file << "toolPickDurability=" << tools.durability(mud::tool::ToolId::Pickaxe) << "\n";

    const auto& bag = player.GetBag();
    const auto* objectsPtr = &bag.GetObjects();
    file << "bagCount=" << objectsPtr->size() << "\n";

    for (const auto* obj : *objectsPtr) {
        file << "item=" << obj->GetName() << "|"
             << obj->GetDescription() << "|"
             << obj->GetHealth() << "|"
             << obj->GetSellingPrice() << "|"
             << obj->GetBuyingPrice() << "|"
             << obj->GetQuantity() << "\n";
    }

    file.close();
    return true;
}

bool PlayerSerializer::Save(const std::string& filename, const Player& player) {
    Game game;
    return Save(filename, player, game, 0, mud::tool::ToolController{}, 0);
}

bool PlayerSerializer::Load(const std::string& filename, Player& player, Game& game, int& gold,
                            mud::tool::ToolController& tools, std::int64_t& totalGameMinutes) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    PositionCode pos = AtHome;
    StateCode state = Waiting;
    int satiety = 100, maxSatiety = 100;
    int farmingExp = 0, fishExp = 0, mineExp = 0;
    int goldInFile = 0;
    bool hasGold = false;
    bool hasTotalMinutes = false;

    long long saveOpenYear = 0, saveOpenMonth = 1, saveOpenDay = 0, saveOpenHour = 0, saveOpenMinute = 0;
    long long totalPlaySeconds = 0;

    int hoeLevel = 1, hoeDurability = 0;
    int rodLevel = 1, rodDurability = 0;
    int pickLevel = 1, pickDurability = 0;
    bool hasTools = false;

    std::string line;
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
            } else if (key == "gold") {
                goldInFile = std::stoi(value);
                hasGold = true;
            } else if (key == "saveOpenYear") {
                saveOpenYear = std::stoll(value);
            } else if (key == "saveOpenMonth") {
                saveOpenMonth = std::stoll(value);
            } else if (key == "saveOpenDay") {
                saveOpenDay = std::stoll(value);
            } else if (key == "saveOpenHour") {
                saveOpenHour = std::stoll(value);
            } else if (key == "saveOpenMinute") {
                saveOpenMinute = std::stoll(value);
            } else if (key == "totalPlaySeconds") {
                totalPlaySeconds = std::stoll(value);
            } else if (key == "gameTotalMinutes") {
                totalGameMinutes = std::stoll(value);
                hasTotalMinutes = true;
            } else if (key == "toolHoeLevel") {
                hoeLevel = std::stoi(value);
                hasTools = true;
            } else if (key == "toolHoeDurability") {
                hoeDurability = std::stoi(value);
                hasTools = true;
            } else if (key == "toolRodLevel") {
                rodLevel = std::stoi(value);
                hasTools = true;
            } else if (key == "toolRodDurability") {
                rodDurability = std::stoi(value);
                hasTools = true;
            } else if (key == "toolPickLevel") {
                pickLevel = std::stoi(value);
                hasTools = true;
            } else if (key == "toolPickDurability") {
                pickDurability = std::stoi(value);
                hasTools = true;
            } else if (key == "bagCount") {
                // 容器连线数（按行还原，不必记录该值）
            } else if (key == "item") {
                itemLines.push_back(value);
            }
        }
    }

    player = Player(pos, state, satiety, maxSatiety, farmingExp, fishExp, mineExp);

    using namespace std::chrono;
    // 由日历字段重建保存开启时间（游戏纪元 0年1月1日 起推算，1970 前视为纪元起点）
    auto ymd = year{static_cast<int>(saveOpenYear)} / month{static_cast<unsigned>(saveOpenMonth)}
             / day{static_cast<unsigned>(std::max<long long>(saveOpenDay, 1))};
    auto saveOpenTp = sys_days{ymd} + hours{static_cast<long long>(std::min<long long>(saveOpenHour, 23))}
                    + minutes{static_cast<long long>(std::min<long long>(saveOpenMinute, 59))};

    if (saveOpenTp.time_since_epoch() >= seconds(0)) {
        game.setSaveOpenTime(system_clock::time_point(saveOpenTp));
    }
    game.setTotalPlayTime(seconds(totalPlaySeconds));

    if (hasGold) gold = goldInFile;
    // 仅当存档含工具状态段时还原；旧存档缺省保持调用方现有工具（满耐久/1级）
    if (hasTools) {
        tools.restore(mud::tool::ToolId::Hoe, hoeLevel, hoeDurability);
        tools.restore(mud::tool::ToolId::Rod, rodLevel, rodDurability);
        tools.restore(mud::tool::ToolId::Pickaxe, pickLevel, pickDurability);
    }
    (void)hasTotalMinutes; // gameTotalMinutes 不存在时仅保持调用方初值

    for (const auto& itemLine : itemLines) {
        std::istringstream itemIss(itemLine);
        std::string name, description, healthStr, sellStr, buyStr;
        if (std::getline(itemIss, name, '|') &&
            std::getline(itemIss, description, '|') &&
            std::getline(itemIss, healthStr, '|') &&
            std::getline(itemIss, sellStr, '|') &&
            std::getline(itemIss, buyStr, '|')) {
            int health = std::stoi(healthStr);
            int sellPrice = std::stoi(sellStr);
            int buyPrice = std::stoi(buyStr);
            Object* obj = new Object(name, description, health, sellPrice, buyPrice);
            // 第 6 段为可选数量段（兼容旧存档）；缺省数量为 1
            std::string qtyStr;
            if (std::getline(itemIss, qtyStr) && !qtyStr.empty()) {
                try { obj->SetQuantity(std::stoi(qtyStr)); } catch (...) { /* 忽略非法数量 */ }
            }
            // 存档已按堆叠聚合：直接入包，避免再次合并
            player.GetBag().AddUnique(obj);
        }
    }

    file.close();
    return true;
}

bool PlayerSerializer::Load(const std::string& filename, Player& player) {
    Game game;
    int gold = 0;
    mud::tool::ToolController tools;
    std::int64_t total = -1;
    return Load(filename, player, game, gold, tools, total);
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
