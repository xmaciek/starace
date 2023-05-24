#pragma once

#include <engine/action.hpp>
#include <shared/fixed_map.hpp>
#include <shared/pmr_pointer.hpp>
#include <shared/pool.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <vector>

class ActionStateTracker {

    struct Pair {
        Actuator m_min{};
        Actuator m_max{};
        Action::Enum m_userEnum{};

        int16_t m_maxF = 0;
        int16_t m_minF = 0;

        Action makeAction() const;
    };

    std::pmr::vector<Pair> m_data{};

public:
    void add( Action::Enum, Actuator );
    void add( Action::Enum, Actuator, Actuator );
    std::pmr::vector<Action> updateAndResolve( Actuator );

};
