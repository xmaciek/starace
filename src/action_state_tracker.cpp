#include "action_state_tracker.hpp"

#include <algorithm>
#include <cassert>

void ActionStateTracker::add( Action::Enum e, Actuator a )
{
    m_actionMap.insert( a, e );
}

void ActionStateTracker::add( Action::Enum e, Actuator a1, Actuator a2 )
{
    uint64_t index = m_indexer.next();
    m_analogActionsArray[ index ] = DigitalToAnalog{ a1, a2, e, false, false };
    m_analogActionsMap.insert( a1, static_cast<uint16_t>( index ) );
    m_analogActionsMap.insert( a2, static_cast<uint16_t>( index ) );
}

Action ActionStateTracker::DigitalToAnalog::makeAction( Actuator a )
{
    switch ( a.typed.type ) {
    case Actuator::SCANCODE:
    case Actuator::BUTTONCODE: {
        if ( m_minA <=> a == 0 ) { m_minD = !!a.value; }
        else if ( m_maxA <=> a == 0 ) { m_maxD = !!a.value; }
        const float ret = ( m_minD == m_maxD ) ? 0.0f : m_minD ? -1.0f : 1.0f;
        return { .analog = ret, .userEnum = m_eid };
    }

    case Actuator::AXISCODE: {
        if ( m_minA <=> a == 0 ) { m_minF = a.value; }
        else if ( m_maxA <=> a == 0 ) { m_maxF = a.value; }
        const int16_t diff = m_maxF - m_minF;
        const float ret = diff > 0
            ? static_cast<float>( diff ) / static_cast<float>( Actuator::MAX )
            : -static_cast<float>( diff ) / static_cast<float>( Actuator::MIN );
        return { .analog = ret, .userEnum = m_eid, };
    }

    assert( !"invalid actuator" );
    default: return {};
    }
}

std::pmr::vector<Action> ActionStateTracker::updateAndResolve( Actuator a )
{
    auto [ begin1, end1 ] = m_actionMap.equalRange( a );
    auto [ begin2, end2 ] = m_analogActionsMap.equalRange( a );

    std::pmr::vector<Action> actions( static_cast<size_t>( std::distance( begin1, end1 ) + std::distance( begin2, end2 ) ) );

    auto it = std::transform( begin1, end1, actions.begin(),
        [ a ]( Action::Enum e ) -> Action
        {
            switch ( a.typed.type ) {
            case Actuator::SCANCODE:
            case Actuator::BUTTONCODE:
                return { .digital = !!a.value, .userEnum = e, };

            case Actuator::AXISCODE: {
                const float v = a.value > 0
                    ? static_cast<float>( a.value ) / static_cast<float>( Actuator::MAX )
                    : -static_cast<float>( a.value ) / static_cast<float>( Actuator::MIN );
                return { .analog = v, .userEnum = e, };
            }
            default:
                assert( !"invalid actuator" );
                return {};
            }
        }
    );

    std::transform( begin2, end2, it,
        [ this, a ]( uint16_t idx )
        {
            return m_analogActionsArray[ idx ].makeAction( a );
        }
    );
    return actions;
}
