#include "bullet.hpp"

#include <cassert>
#include <random>

static uint16_t typeToSegments( GLuint t )
{
    switch ( t ) {
    case Bullet::TORPEDO:
        return 24;
    case Bullet::BLASTER:
        return 9;
    case Bullet::SLUG:
        return 2;
    default:
        return 0;
    }
}

Bullet::Bullet( const BulletProto& bp )
: m_tail( typeToSegments( bp.type ), Vertex( bp.x, bp.y, bp.z ) )
{
    CollisionFlag = true;
    m_type = bp.type;
    speed = bp.speed;
    m_damage = bp.damage;
    std::copy( std::begin( bp.color1 ), std::end( bp.color1 ), std::begin( m_color1 ) );
    std::copy( std::begin( bp.color2 ), std::end( bp.color2 ), std::begin( m_color2 ) );
    score = bp.score_per_hit;

    if ( m_type == SLUG ) {
        std::copy( std::begin( m_color1 ), std::end( m_color1 ), std::begin( m_color2 ) );
        m_color1[ 3 ] = 1;
        m_color2[ 3 ] = 0;
    }
    position.x = bp.x;
    position.y = bp.y;
    position.z = bp.z;
    turnrate_in_rads = ( speed * 10 ) * DEG2RAD * DELTATIME;
    m_rotX = 0;
    m_rotY = 0;
    m_rotZ = 0;
    static std::mt19937_64 rng{ std::random_device()() };
    m_rotation = rng() % 360;

    m_range = 0;
    m_maxRange = 150;

    status = ALIVE;
    ttl = 20;
};

void Bullet::Draw1() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( static_cast<const GLfloat*>( m_color1 ) );
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

void Bullet::DrawLaser() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glBegin( GL_LINES );
    glColor4fv( static_cast<const GLfloat*>( m_color1 ) );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it++;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glPopMatrix();
}

void Bullet::Draw2() const
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( static_cast<const GLfloat*>( m_color1 ) );
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

void Bullet::Draw() const
{
    if ( status == DEAD ) {
        return;
    }
    switch ( m_type ) {
    case SLUG:
        DrawLaser();
        break;
    case BLASTER:
        Draw1();
        break;
    case TORPEDO:
        Draw2();
        break;
    default:
        break;
    }
};

void Bullet::Update()
{
    if ( status == DEAD ) {
        return;
    }
    if ( m_range > m_maxRange ) {
        status = DEAD;
        return;
    }

    switch ( m_type ) {
    case SLUG:
        m_color1[ 3 ] -= 2.0 * DELTATIME;
        m_range += m_maxRange * 2.0 * DELTATIME;
        break;
    case TORPEDO:
        InterceptTarget(); /*no break*/
    case BLASTER:
        position = position + velocity * DELTATIME;
        m_tail.insert( position );
        m_range += speed * DELTATIME;
        break;
    default:
        break;
    }
};

bool Bullet::collisionTest( const SAObject* object ) const
{
    assert( object );
    if ( !object->CanCollide() || status == DEAD || object->GetStatus() != ALIVE ) {
        return false;
    }

    const Vertex collRay = collisionRay();
    const Vertex dir = object->GetPosition() - position;
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
            const Vertex Pb = position + ( collRay * ( tmp / tmp2 ) );
            dist = lengthV( object->GetPosition() - Pb );
        }
    }

    return dist < ( CollisionDistance + object->GetCollisionDistance() );
}

void Bullet::ProcessCollision( SAObject* object )
{
    assert( object );
    if ( collisionTest( object ) ) {
        object->AddScore( score, false );
        switch ( m_type ) {
        case SLUG:
            object->Damage( m_damage * m_color1[ 3 ] );
            break;
        default:
            object->Damage( m_damage );
            status = DEAD;
            break;
        }
    }
}

Vertex Bullet::collisionRay() const
{
    switch ( m_type ) {
    case BLASTER:
        return direction * speed * DELTATIME * 3;
    case TORPEDO:
        return direction * speed * DELTATIME;
    case SLUG:
        return direction * 1000;
    default:
        assert( !"unreachable" );
        return Vertex{};
    }
}

void Bullet::SetDirection( Vertex v )
{
    direction = v;
    normalizeV( direction );
    velocity = direction * speed;
    if ( m_type == SLUG ) {
        m_tail.insert( position + ( direction * 1000 ) );
    }
}

GLuint Bullet::getDamage() const
{
    return m_damage;
}

GLuint Bullet::GetType() const
{
    return m_type;
}
