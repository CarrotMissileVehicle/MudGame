//
// Created by opencode on 2026/9/1.
//

#ifndef MUDGAME_PLAYERSERIALIZER_H
#define MUDGAME_PLAYERSERIALIZER_H

#include <cstdint>
#include <string>
#include "../../Controller/Player/include/Player.h"
#include "../../Controller/Game/include/Game.h"
#include "../../Controller/Tool/include/tool_controller.h"

class PlayerSerializer {
public:
    /**
     * @brief 读档结果（DEF-385）：从 bool 升级为显式状态，调用方可区分
     *        文件不存在 / 解析失败 / 成功，而非全部坍缩为一个 false。
     */
    enum class LoadStatus {
        Ok,                  // 成功：输出参数已整体提交
        NotFound,            // 文件不存在或无法打开（权限/路径问题）
        Corrupt,             // 内容损坏无法解析（当前实现按字段容错，预留）
        UnsupportedVersion,  // 存档版本不受支持（预留）
    };

    // 完整存档：玩家 + 游戏会话 + 金币 + 工具状态 + 游戏内时钟总分钟数
    // 金币为 64 位（DEF-007：与 Market/main 的 long long gold 对齐）
    bool Save(const std::string& filename, const Player& player, const Game& game, long long gold,
              const mud::tool::ToolController& tools, std::int64_t totalGameMinutes);

    /**
     * @brief 读档（commit-on-success）：全部字段先解析到局部变量，
     *        文件整体解析完成才写入输出参数；失败时输出参数保持不变。
     */
    LoadStatus Load(const std::string& filename, Player& player, Game& game, long long& gold,
                    mud::tool::ToolController& tools, std::int64_t& totalGameMinutes);

    bool Save(const std::string& filename, const Player& player);
    LoadStatus Load(const std::string& filename, Player& player);

private:
    std::string GetPositionName(PositionCode code) const;
    std::string GetStateName(StateCode code) const;
    PositionCode ParsePosition(const std::string& name) const;
    StateCode ParseState(const std::string& name) const;
};

#endif //MUDGAME_PLAYERSERIALIZER_H
