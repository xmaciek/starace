#include "saobject.hpp"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

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

glm::vec3 SAObject::position() const
{
    return m_position;
}

glm::vec3 SAObject::direction() const
{
    return m_direction;
}

glm::vec3 SAObject::velocity() const
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

void SAObject::processCollision( SAObject* ) { }

void SAObject::setTarget( SAObject* t )
{
    m_target = t;
}

void SAObject::targetMe( bool b )
{
    m_isTargeted = b;
}

void SAObject::kill()
{
    setStatus( Status::eDead );
}

void SAObject::setDamage( double d )
{
    m_health -= d;
    if ( m_health <= 0.0 ) {
        setStatus( Status::eDead );
    }
}

double SAObject::health() const
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

    glm::vec3 dir = m_direction;
    glm::vec3 tgt = m_target->position();
    tgt = m_position - tgt;
    tgt = glm::normalize( tgt );

    const float tmp = (float)std::tan( std::atan( glm::dot( dir, tgt ) - m_turnrate ) );
    dir = glm::cross( tgt, glm::cross( dir, tgt ) );
    dir += tgt * glm::vec3( tmp );

    dir = glm::normalize( dir );
    m_direction = dir;
    m_velocity = m_direction * glm::vec3( m_speed );
}

bool SAObject::canCollide() const
{
    return m_collisionFlag;
}

double SAObject::collisionDistance() const
{
    return m_collisionDistance;
}

double SAObject::collisionDamage() const
{
    return m_collisionDamage;
}

bool SAObject::deleteMe()
{
    if ( m_ttl == 0 ) {
        return true;
    }
    m_ttl--;
    return false;
}

uint32_t SAObject::score() const
{
    return m_score;
}

void SAObject::addScore( uint32_t s, bool )
{
    m_score += s;
}
