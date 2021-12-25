#pragma once

#include <engine/action.hpp>

#include <shared/fixed_map.hpp>
#include <shared/pool.hpp>

#include <vector>
#include <memory_resource>

class ActionMapping {
private:
    struct DigitalToAnalog {
        Actuator m_minA{};
        Actuator m_maxA{};
        Action::Enum m_eid{};
        bool m_min : 1 = false;
        bool m_max : 1 = false;
        float value( Actuator a, bool state )
        {
            if ( m_minA <=> a == 0 ) { m_min = state; }
            if ( m_maxA <=> a == 0 ) { m_max = state; }
            return ( m_min == m_max ) ? 0.0f :
                m_min ? -1.0f : 1.0f;
        }
    };

    Pool<DigitalToAnalog, 32> m_analogActionsPool{};
    FixedMap<Actuator, Action::Enum, 32> m_actionMap{};
    FixedMap<Actuator, DigitalToAnalog*, 32> m_analogActions{};

public:
    using Range1 = std::tuple<const Action::Enum*, const Action::Enum*>;
    using Range2 = std::pmr::vector<std::tuple<Action::Enum, float>>;

    ActionMapping() noexcept = default;

    void registerAction( Action::Enum, Actuator );
    void registerAction( Action::Enum, Actuator, Actuator );

    Range1 resolve1( Actuator ) const;
    Range2 resolve2( Actuator, bool );
};
