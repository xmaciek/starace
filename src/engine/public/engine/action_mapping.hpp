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
        bool m_minD : 1 = false;
        bool m_maxD : 1 = false;
        int16_t m_minF = 0;
        int16_t m_maxF = 0;
        inline float value( Actuator a, bool state )
        {
            if ( m_minA <=> a == 0 ) { m_minD = state; }
            else if ( m_maxA <=> a == 0 ) { m_maxD = state; }
            return ( m_minD == m_maxD ) ? 0.0f :
                m_minD ? -1.0f : 1.0f;
        }
        inline float value( Actuator a, int16_t state )
        {
            if ( m_minA <=> a == 0 ) { m_minF = state; }
            else if ( m_maxA <=> a == 0 ) { m_maxF = state; }
            const int16_t diff = m_maxF - m_minF;
            return static_cast<float>( diff ) / static_cast<float>( 0x7FFFu );
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
    Range2 resolve2( Actuator, int16_t );
};
