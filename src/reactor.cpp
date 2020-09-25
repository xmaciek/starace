#include "reactor.hpp"

#include <algorithm>

bool Reactor::consume( float amount )
{
    if ( m_current < amount ) {
        return false;
    }
    m_current -= amount;
    return true;
}

float Reactor::power() const
{
    return m_current;
}

void Reactor::update( const UpdateContext& updateContext )
{
    m_current = std::min( m_current + m_generation * updateContext.deltaTime, m_capacity );
}
