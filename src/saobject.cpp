#include "saobject.hpp"

#include <cmath>

float SAObject::x() const
{
    return m_position.x;
}

float SAObject::y() const
{
    return m_position.y;
}

float SAObject::z() const
{
    return m_position.z;
}

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
    return m_velocity;
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

void SAObject::interceptTarget()
{
    if ( !m_target ) {
        return;
    }
    if ( m_target->status() != Status::eAlive ) {
        m_target = nullptr;
        return;
    }

    math::vec3 dir = m_direction;
    math::vec3 tgt = m_target->position();
    tgt = m_position - tgt;
    tgt = math::normalize( tgt );

    const float tmp = (float)std::tan( std::atan( math::dot( dir, tgt ) - m_turnrate ) );
    dir = math::cross( tgt, math::cross( dir, tgt ) );
    dir += tgt * math::vec3( tmp );

    dir = math::normalize( dir );
    m_direction = dir;
    m_velocity = m_direction * math::vec3( m_speed );
}

uint32_t SAObject::score() const
{
    return m_score;
}

void SAObject::addScore( uint32_t s, bool )
{
    m_score += s;
}

void SAObject::update( const UpdateContext& )
{
    m_health -= std::min<uint16_t>( m_health, m_pendingDamage );
    m_pendingDamage = 0;
    if ( m_health == 0 ) {
        setStatus( Status::eDead );
    }
}

SAObject* SAObject::target() const
{
    return m_target;
}
