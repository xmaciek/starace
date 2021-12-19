#include <engine/action_mapping.hpp>

void ActionMapping::registerAction( Action::Enum eid, Actuator a )
{
    m_actionMap.insert( a, eid );
}

ActionMapping::Range ActionMapping::resolve( Actuator a ) const
{
    return m_actionMap.equalRange( a );
}
