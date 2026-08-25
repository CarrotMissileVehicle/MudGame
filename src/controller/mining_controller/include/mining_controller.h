#pragma once

#include "mining_state.h"
#include "mining_types.h"

using namespace mud::mining;

#include <cstddef>
#include <vector>

class OreData;
class TimeService;

class MiningController
{
public:
    MiningController(
        const OreData& ore_data,
        const TimeService& time_service
    );

    bool start_mining(
        MiningState& state,
        std::size_t layer_id,
        const MiningContext& context
    );

    std::vector<MiningResult> update(
        MiningState& state,
        const MiningContext& context
    );

    std::vector<MiningResult> stop_mining(
        MiningState& state,
        const MiningContext& context
    );

    bool is_mining(
        const MiningState& state
    ) const noexcept;

private:
    bool can_enter_layer(
        std::size_t layer_id,
        const MiningContext& context
    ) const;

    bool check_lighting(
        std::size_t layer_id,
        const MiningContext& context
    ) const;

    std::vector<MiningResult> produce(
        std::size_t layer_id,
        std::size_t count,
        const MiningContext& context
    ) const;

    std::string random_ore(
        std::size_t layer_id,
        const MiningContext& context
    ) const;

private:
    const OreData& ore_data_;
    const TimeService& time_service_;
};