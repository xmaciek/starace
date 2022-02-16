#include "saobject.hpp"

#include <cmath>

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

uint8_t SAObject::health() const
{
    return m_health;
}

void SAObject::update( const UpdateContext& )
{
    m_health = static_cast<uint8_t>( std::max( m_health - m_pendingDamage, 0 ) );
    m_pendingDamage = 0;
    if ( m_health == 0 ) {
        setStatus( Status::eDead );
    }
}

SAObject* SAObject::target() const
{
    return m_target;
}
