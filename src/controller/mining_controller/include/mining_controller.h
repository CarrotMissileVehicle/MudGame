#pragma once

#include "mining_state.h"
#include "mining_types.h"
#include "ore_data.h"

#include "time_service.h"

using namespace mud;

#include <cstddef>
#include <vector>

class MiningController
{
public:
    MiningController(
        const Ore::OreData& ore_data,
        const TimeService& time_service
    );

    bool start_mining(
        MiningState& state,
        std::size_t layer_id,
        const mining::MiningContext& context
    );

    std::vector<mining::MiningResult> update(
        MiningState& state,
        const mining::MiningContext& context
    );

    std::vector<mining::MiningResult> stop_mining(
        MiningState& state,
        const mining::MiningContext& context
    );

    bool is_mining(
        const MiningState& state
    ) const noexcept;

private:
    bool can_enter_layer(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

    bool check_lighting(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

    std::vector<mining::MiningResult> produce(
        std::size_t layer_id,
        std::size_t count,
        const mining::MiningContext& context
    ) const;

    std::string random_ore(
        std::size_t layer_id,
        const mining::MiningContext& context
    ) const;

private:
    const Ore::OreData& ore_data_;
    const TimeService& time_service_;
};