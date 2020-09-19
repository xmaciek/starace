#include "saobject.hpp"

GLdouble SAObject::getX() const
{
    return position.x;
}

GLdouble SAObject::getY() const
{
    return position.y;
}

GLdouble SAObject::getZ() const
{
    return position.z;
}

GLuint SAObject::GetStatus() const
{
    return status;
}

Vertex SAObject::GetPosition() const
{
    return position;
}

Vertex SAObject::GetDirection() const
{
    return direction;
}

Vertex SAObject::GetVelocity() const
{
    return velocity;
}

GLdouble SAObject::GetSpeed() const
{
    return speed;
}

void SAObject::SetStatus( GLuint s )
{
    status = s;
}

void SAObject::ProcessCollision( SAObject* ) { }

void SAObject::SetTarget( SAObject* t )
{
    target = t;
}

void SAObject::TargetMe( bool doit )
{
    ImTargeted = doit;
}

void SAObject::Kill()
{
    status = DEAD;
}

void SAObject::Damage( GLdouble d )
{
    health -= d;
    if ( health <= 0 ) {
        status = DEAD;
    }
}

GLdouble SAObject::GetHealth() const
{
    return health;
}

void SAObject::InterceptTarget()
{
    if ( !target ) {
        return;
    }
    if ( target->GetStatus() != ALIVE ) {
        target = nullptr;
        return;
    }

    Vertex D = direction;
    Vertex T = target->GetPosition();
    T = position - T;
    normalise_v( T );

    const double tmp = tan( atan( dot_product( D, T ) - turnrate_in_rads ) );
    D = cross_product( T, cross_product( D, T ) );
    D = D + ( T * tmp );

    normalise_v( D );
    direction = D;
    velocity = direction * speed;
}

bool SAObject::CanCollide() const
{
    return CollisionFlag;
}

GLdouble SAObject::GetCollisionDistance() const
{
    return CollisionDistance;
}

GLdouble SAObject::GetCollisionDamage() const
{
    return CollisionDamage;
}

bool SAObject::DeleteMe()
{
    if ( ttl == 0 ) {
        return true;
    }
    ttl--;
    return false;
}

GLint SAObject::GetScore() const
{
    return score;
}

void SAObject::AddScore( GLint s, bool )
{
    score += s;
}
