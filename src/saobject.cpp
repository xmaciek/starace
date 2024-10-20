#include "saobject.hpp"

#include <algorithm>

SAObject::Status SAObject::status() const
{
    return m_status;
}

math::vec3 SAObject::position() const
{
    return m_position;
}

math::vec3 SAObject::direction() const
{
    return m_direction;
}

math::vec3 SAObject::velocity() const
{
    return m_direction * m_speed;
}

float SAObject::speed() const
{
    return m_speed;
}

void SAObject::setStatus( SAObject::Status s )
{
    m_status = s;
}

void SAObject::setPosition( const math::vec3& v )
{
    m_position = v;
}

void SAObject::setTarget( SAObject* t )
{
    m_target = t;
}

void SAObject::kill()
{
    setStatus( Status::eDead );
}

void SAObject::setDamage( uint8_t d )
{
    m_pendingDamage += d;
}

uint16_t SAObject::health() const
{
    return m_health;
}

Signal SAObject::scanSignals( math::vec3 position, std::span<const Signal> signals )
{
    if ( signals.empty() ) return {};

    auto proc = [position]( const Signal& lhs, const Signal& rhs )
    {
        return math::distance( position, lhs.position ) < math::distance( position, rhs.position );
    };
    return *std::min_element( signals.begin(), signals.end(), std::move( proc ) );
}

SAObject* SAObject::target() const
{
    return m_target;
}
