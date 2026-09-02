/**
 * @file mining_controller.h
 * @brief 采矿控制器（MiningController）。
 *
 * 负责采矿会话的启动、进度更新与停止，并产出 MineralResult。
 * 通过 MiningState 维护会话状态，结合 Ore::OreData 生成随机矿石产出。
 *
 * 依赖：mining_state、mining_types、ore_data、time_service。
 */
#pragma once

#include "mining_state.h"
#include "mining_types.h"
#include "ore_data.h"

#include "time_service.h"

using namespace mud;

#include <cstddef>
#include <vector>

/** @brief 采矿流程控制器：驱动会话状态与产出计算。 */
class MiningController
{
public:
    /**
     * @brief 构造采矿控制器。
     * @param ore_data     矿石数据源。
     * @param time_service 时间服务，用于推进时间。
     */
    MiningController(
        const Ore::OreData& ore_data,
        const TimeService& time_service
    );

    /**
     * @brief 启动一次采矿会话。
     * @param state   会话状态（就地修改）。
     * @param layer_id 目标层。
     * @param context  玩家采矿上下文。
     * @return 启动成功返回 true；层不可达或照明不足返回 false。
     */
    bool start_mining(
        MiningState& state,
        std::size_t layer_id,
        const mining::MiningContext& context
    );

    /**
     * @brief 推进采矿进度，一次返回到期应产出的结果。
     * @param state   会话状态（就地修改）。
     * @param context 玩家采矿上下文。
     * @return 本次产出结果列表；未到期则为空。
     */
    std::vector<mining::MiningResult> update(
        MiningState& state,
        const mining::MiningContext& context
    );

    /**
     * @brief 停止采矿并结算未提取的产出。
     * @param state   会话状态（就地修改为 Idle）。
     * @param context 玩家采矿上下文。
     * @return 结算得到的产出结果列表。
     */
    std::vector<mining::MiningResult> stop_mining(
        MiningState& state,
        const mining::MiningContext& context
    );

    /** @brief 查询指定会话是否处于采矿中。 */
    bool is_mining(
        const MiningState& state
    ) const noexcept;

private:
    /** @brief 检查玩家是否满足进入目标层的等级与照明条件。 */
    bool can_enter_layer(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

    /** @brief 检查照明条件是否满足目标层需求。 */
    bool check_lighting(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

    /** @brief 生成 count 次采矿的产出结果列表。 */
    std::vector<mining::MiningResult> produce(
        std::size_t layer_id,
        std::size_t count,
        const mining::MiningContext& context
    ) const;

    /** @brief 依据目标层产出分布随机抽取一种矿石 ID。 */
    std::string random_ore(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

private:
    const Ore::OreData& ore_data_;    // 矿石数据源（外部所有）
    const TimeService& time_service_; // 时间服务（外部所有）
};