#include "action_state_tracker.hpp"

#include <algorithm>

void ActionStateTracker::add( Action::Enum e, Actuator a )
{
    m_data.push_back( Pair{ .m_max = a, .m_userEnum = e } );
}

void ActionStateTracker::add( Action::Enum e, Actuator a1, Actuator a2 )
{
    m_data.push_back( Pair{ .m_min = a1, .m_max = a2, .m_userEnum = e } );
}

std::pmr::vector<Action> ActionStateTracker::updateAndResolve( Actuator a )
{
    std::pmr::vector<Action> ret;
    for ( auto& it : m_data ) {
        if ( it.m_max == a ) {
            it.m_maxF = a.value;
            ret.emplace_back( it.makeAction() );
        }
        else if ( it.m_min == a ) {
            it.m_minF = a.value;
            ret.emplace_back( it.makeAction() );
        }
    }
    return ret;
}

Action ActionStateTracker::Pair::makeAction() const
{
    int32_t vmax = m_maxF;
    int32_t vmin = -(int32_t)m_minF;
    int32_t value = std::clamp<int32_t>( vmax + vmin, Actuator::MIN, Actuator::MAX );
    return Action{ .userEnum = m_userEnum, .value = static_cast<Actuator::value_type>( value ) };
}
