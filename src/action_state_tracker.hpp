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

    struct DigitalToAnalog {
        Actuator m_minA{};
        Actuator m_maxA{};
        Action::Enum m_eid{};
        bool m_minD : 1 = false;
        bool m_maxD : 1 = false;
        int16_t m_minF = 0;
        int16_t m_maxF = 0;

        Action makeAction( Actuator a );
    };

    FixedMap<Actuator, Action::Enum, 32> m_actionMap{};

    static constexpr uint64_t c_analogCount = 16;
    std::array<DigitalToAnalog, c_analogCount> m_analogActionsArray{};
    Indexer<c_analogCount> m_indexer{};
    FixedMap<Actuator, uint16_t, c_analogCount> m_analogActionsMap{};

public:
    void add( Action::Enum, Actuator );
    void add( Action::Enum, Actuator, Actuator );
    std::pmr::vector<Action> updateAndResolve( Actuator );

};
