#include "bullet.hpp"

#include <cassert>
#include <random>

static uint16_t typeToSegments( Bullet::Type e )
{
    switch ( e ) {
    case Bullet::Type::eTorpedo:
        return 24;
    case Bullet::Type::eBlaster:
        return 9;
    case Bullet::Type::eSlug:
        return 2;
    default:
        return 0;
    }
}

Bullet::Bullet( const BulletProto& bp )
: m_tail( typeToSegments( bp.type ), Vertex( bp.x, bp.y, bp.z ) )
{
    m_collisionFlag = true;
    m_type = bp.type;
    m_speed = bp.speed;
    m_damage = bp.damage;
    std::copy( std::begin( bp.color1 ), std::end( bp.color1 ), std::begin( m_color1 ) );
    std::copy( std::begin( bp.color2 ), std::end( bp.color2 ), std::begin( m_color2 ) );
    m_score = bp.score_per_hit;

    if ( m_type == Type::eSlug ) {
        std::copy( std::begin( m_color1 ), std::end( m_color1 ), std::begin( m_color2 ) );
        m_color1[ 3 ] = 1;
        m_color2[ 3 ] = 0;
    }
    m_position.x = bp.x;
    m_position.y = bp.y;
    m_position.z = bp.z;
    m_turnrate = ( m_speed * 10 ) * DEG2RAD * DELTATIME;
    m_rotX = 0;
    m_rotY = 0;
    m_rotZ = 0;
    static std::mt19937_64 rng{ std::random_device()() };
    m_rotation = rng() % 360;

    m_range = 0;
    m_maxRange = 150;

    setStatus( Status::eAlive );
    m_ttl = 20;
};

void Bullet::draw1() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( static_cast<const float*>( m_color1 ) );
    glBegin( GL_LINES );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it += 3;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glBegin( GL_LINES );
    glColor4f( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], 1 );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it += 5;
    glColor4f( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], 0 );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glPopMatrix();
};

void Bullet::drawLaser() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glBegin( GL_LINES );
    glColor4fv( static_cast<const float*>( m_color1 ) );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it++;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glPopMatrix();
}

void Bullet::draw2() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( static_cast<const float*>( m_color1 ) );
    glBegin( GL_LINES );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it++;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glColor4f( 1, 1, 1, 1 );
    uint16_t alphaIt = 1;
    const Tail::const_iterator end = m_tail.end();
    glBegin( GL_LINE_STRIP );
    while ( it != end ) {
        glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
        glColor4f( 1, 1, 1, 1.0 / alphaIt );
        alphaIt++;
        it++;
    }
    glEnd();

    glPopMatrix();
};

void Bullet::draw() const
{
    if ( status() == Status::eDead ) {
        return;
    }

    switch ( m_type ) {
    case Type::eSlug:
        drawLaser();
        break;

    case Type::eBlaster:
        draw1();
        break;

    case Type::eTorpedo:
        draw2();
        break;
    }
};

void Bullet::update()
{
    if ( status() == Status::eDead ) {
        return;
    }
    if ( m_range > m_maxRange ) {
        setStatus( Status::eDead );
        return;
    }

    switch ( m_type ) {
    case Type::eSlug:
        m_color1[ 3 ] -= 2.0 * DELTATIME;
        m_range += m_maxRange * 2.0 * DELTATIME;
        break;

    case Type::eTorpedo:
        interceptTarget();
        /*no break*/

    case Type::eBlaster:
        m_position += m_velocity * DELTATIME;
        m_tail.insert( m_position );
        m_range += m_speed * DELTATIME;
        break;

    }
};

bool Bullet::collisionTest( const SAObject* object ) const
{
    assert( object );
    if ( !object->canCollide() || status() == Status::eDead || object->status() != Status::eAlive ) {
        return false;
    }

    const Vertex collRay = collisionRay();
    const Vertex dir = object->position() - position();
    const double tmp = dotProduct( dir, collRay );
    double dist = 0.0;

    if ( tmp <= 0 ) {
        dist = lengthV( dir );
    }
    else {
        const double tmp2 = dotProduct( collRay, collRay );
        if ( tmp2 <= tmp ) {
            dist = lengthV( dir );
        }
        else {
            const Vertex Pb = position() + ( collRay * ( tmp / tmp2 ) );
            dist = lengthV( object->position() - Pb );
        }
    }

    return dist < ( m_collisionDistance + object->collisionDistance() );
}

void Bullet::processCollision( SAObject* object )
{
    assert( object );
    if ( collisionTest( object ) ) {
        object->addScore( score(), false );
        switch ( m_type ) {
        case Type::eSlug:
            object->setDamage( m_damage * m_color1[ 3 ] );
            break;
        default:
            object->setDamage( m_damage );
            setStatus( Status::eDead );
            break;
        }
    }
}

Vertex Bullet::collisionRay() const
{
    switch ( m_type ) {
    case Type::eBlaster:
        return direction() * speed() * DELTATIME * 3;

    case Type::eTorpedo:
        return direction() * speed() * DELTATIME;

    case Type::eSlug:
        return direction() * 1000;

    default:
        assert( !"unreachable" );
        return Vertex{};
    }
}

void Bullet::setDirection( const Vertex& v )
{
    m_direction = v;
    normalizeV( m_direction );
    m_velocity = direction() * speed();
    if ( m_type == Type::eSlug ) {
        m_tail.insert( position() + ( direction() * 1000 ) );
    }
}

uint32_t Bullet::damage() const
{
    return m_damage;
}

Bullet::Type Bullet::type() const
{
    return m_type;
}
