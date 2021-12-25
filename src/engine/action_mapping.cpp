#include <engine/action_mapping.hpp>

#include <algorithm>

void ActionMapping::registerAction( Action::Enum eid, Actuator a )
{
    m_actionMap.insert( a, eid );
}

void ActionMapping::registerAction( Action::Enum eid, Actuator a, Actuator b )
{
    void* ptr = m_analogActionsPool.alloc();
    DigitalToAnalog* dta = ::new ( ptr ) DigitalToAnalog{ a, b, eid, false, false };
    m_analogActions.insert( a, dta );
    m_analogActions.insert( b, dta );
}

ActionMapping::Range1 ActionMapping::resolve1( Actuator a ) const
{
    return m_actionMap.equalRange( a );
}

ActionMapping::Range2 ActionMapping::resolve2( Actuator a, bool state )
{
    auto [ begin, end ] = m_analogActions.equalRange( a );
    if ( begin == end ) {
        return {};
    }

    const size_t size = static_cast<size_t>( end - begin );
    Range2 ret{ size };
    std::transform( begin, end, ret.begin(), [a, state](DigitalToAnalog* ptr) -> Range2::value_type
    {
        return { ptr->m_eid, ptr->value( a, state ) };
    } );
    return ret;
}
