//
// Created by opencode on 2026/9/1.
//

#include "../include/PlayerSerializer.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{

// DEF-104：存档为不可信输入——数值字段损坏时回退默认值，
// 而非令 stoi/stoll 异常逃逸导致读档崩溃
int to_int_or(const std::string& text, int fallback) noexcept
{
    try { return std::stoi(text); } catch (...) { return fallback; }
}

long long to_ll_or(const std::string& text, long long fallback) noexcept
{
    try { return std::stoll(text); } catch (...) { return fallback; }
}

} // namespace

bool PlayerSerializer::Save(const std::string& filename, const Player& player, const Game& game,
                            long long gold, const mud::tool::ToolController& tools,
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
    const auto& objects = bag.GetObjects();
    file << "bagCount=" << objects.size() << "\n";

    for (const auto& obj : objects) {
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

bool PlayerSerializer::Load(const std::string& filename, Player& player, Game& game, long long& gold,
                            mud::tool::ToolController& tools, std::int64_t& totalGameMinutes) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    PositionCode pos = AtHome;
    StateCode state = Waiting;
    int satiety = 100, maxSatiety = 100;
    int farmingExp = 0, fishExp = 0, mineExp = 0;
    long long goldInFile = 0;   // DEF-007：金币 64 位，支持大额数值
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
                satiety = to_int_or(value, satiety);
            } else if (key == "maxSatiety") {
                maxSatiety = to_int_or(value, maxSatiety);
            } else if (key == "farmingExp") {
                farmingExp = to_int_or(value, farmingExp);
            } else if (key == "fishExp") {
                fishExp = to_int_or(value, fishExp);
            } else if (key == "mineExp") {
                mineExp = to_int_or(value, mineExp);
            } else if (key == "gold") {
                // DEF-104：金币行损坏视为无该段，保持调用方初值
                try { goldInFile = std::stoll(value); hasGold = true; }
                catch (...) { /* 忽略非法金币 */ }
            } else if (key == "saveOpenYear") {
                saveOpenYear = to_ll_or(value, saveOpenYear);
            } else if (key == "saveOpenMonth") {
                saveOpenMonth = to_ll_or(value, saveOpenMonth);
            } else if (key == "saveOpenDay") {
                saveOpenDay = to_ll_or(value, saveOpenDay);
            } else if (key == "saveOpenHour") {
                saveOpenHour = to_ll_or(value, saveOpenHour);
            } else if (key == "saveOpenMinute") {
                saveOpenMinute = to_ll_or(value, saveOpenMinute);
            } else if (key == "totalPlaySeconds") {
                totalPlaySeconds = to_ll_or(value, totalPlaySeconds);
            } else if (key == "gameTotalMinutes") {
                // DEF-104：损坏视为无该段，保持调用方初值
                try { totalGameMinutes = std::stoll(value); hasTotalMinutes = true; }
                catch (...) { /* 忽略非法总分钟 */ }
            } else if (key == "toolHoeLevel") {
                hoeLevel = to_int_or(value, hoeLevel);
                hasTools = true;
            } else if (key == "toolHoeDurability") {
                hoeDurability = to_int_or(value, hoeDurability);
                hasTools = true;
            } else if (key == "toolRodLevel") {
                rodLevel = to_int_or(value, rodLevel);
                hasTools = true;
            } else if (key == "toolRodDurability") {
                rodDurability = to_int_or(value, rodDurability);
                hasTools = true;
            } else if (key == "toolPickLevel") {
                pickLevel = to_int_or(value, pickLevel);
                hasTools = true;
            } else if (key == "toolPickDurability") {
                pickDurability = to_int_or(value, pickDurability);
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
    // DEF-105：非法日历（月=0/13、日=32、2月30 等）令 sys_days{ymd} 为 UB，
    // 以 ymd.ok() 拦截——非法时保持 game 既有存档时间（与 1970 前分支同语义）
    const auto ymd = year{static_cast<int>(std::clamp<long long>(saveOpenYear, 0, 9999))}
                   / month{static_cast<unsigned>(saveOpenMonth)}
                   / day{static_cast<unsigned>(std::max<long long>(saveOpenDay, 1))};
    if (ymd.ok()) {
        const auto saveOpenTp = sys_days{ymd}
            + hours{std::clamp<long long>(saveOpenHour, 0, 23)}
            + minutes{std::clamp<long long>(saveOpenMinute, 0, 59)};
        if (saveOpenTp.time_since_epoch() >= seconds(0)) {
            game.setSaveOpenTime(system_clock::time_point(saveOpenTp));
        }
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
            int health = to_int_or(healthStr, 0);
            int sellPrice = to_int_or(sellStr, 0);
            int buyPrice = to_int_or(buyStr, 0);
            Object* obj = new Object(name, description, health, sellPrice, buyPrice);
            // 第 6 段为可选数量段（兼容旧存档）；缺省/非法数量回退 1
            std::string qtyStr;
            if (std::getline(itemIss, qtyStr) && !qtyStr.empty()) {
                obj->SetQuantity(to_int_or(qtyStr, 1));
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
    long long gold = 0;
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
