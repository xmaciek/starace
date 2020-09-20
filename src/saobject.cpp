#include "saobject.hpp"

#include <cmath>

GLdouble SAObject::x() const
{
    return m_position.x;
}

GLdouble SAObject::y() const
{
    return m_position.y;
}

GLdouble SAObject::z() const
{
    return m_position.z;
}

SAObject::Status SAObject::status() const
{
    return m_status;
}

Vertex SAObject::position() const
{
    return m_position;
}

Vertex SAObject::direction() const
{
    return m_direction;
}

Vertex SAObject::velocity() const
{
    return m_velocity;
}

GLdouble SAObject::speed() const
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

void SAObject::setDamage( GLdouble d )
{
    m_health -= d;
    if ( m_health <= 0.0 ) {
        setStatus( Status::eDead );
    }
}

GLdouble SAObject::health() const
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

    Vertex dir = m_direction;
    Vertex tgt = m_target->position();
    tgt = m_position - tgt;
    normalizeV( tgt );

    const double tmp = tan( std::atan( dotProduct( dir, tgt ) - m_turnrate ) );
    dir = crossProduct( tgt, crossProduct( dir, tgt ) );
    dir += tgt * tmp;

    normalizeV( dir );
    m_direction = dir;
    m_velocity = m_direction * m_speed;
}

bool SAObject::canCollide() const
{
    return m_collisionFlag;
}

GLdouble SAObject::collisionDistance() const
{
    return m_collisionDistance;
}

GLdouble SAObject::collisionDamage() const
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

GLint SAObject::score() const
{
    return m_score;
}

void SAObject::addScore( GLint s, bool )
{
    m_score += s;
}
