#include "bullet.hpp"

#include <cassert>

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

Bullet::Bullet( BulletProto bp )
: m_tail( typeToSegments( bp.type ), Vertex( bp.x, bp.y, bp.z ) )
{
    //     cout<<"Creating bullet.\n";
    CollisionDistance = 0;
    CollisionFlag = true;
    target = NULL;
    type = bp.type;
    speed = bp.speed;
    damage = bp.damage;
    //     length = bp.length;
    memcpy( color1, bp.color1, sizeof( GLfloat ) * 4 );
    memcpy( color2, bp.color2, sizeof( GLfloat ) * 4 );
    score = bp.score_per_hit;

    if ( type == SLUG ) {
        memcpy( color2, color1, sizeof( GLfloat ) * 4 );
        color1[ 3 ] = 1;
        color2[ 3 ] = 0;
    }
    //     texture1 = bp.texture1;
    //     texture2 = bp.texture2;
    //     std::cout<<texture1;
    target = NULL;
    position.x = bp.x;
    position.y = bp.y;
    position.z = bp.z;
    turnrate_in_rads = ( speed * 10 ) * DEG2RAD * DELTATIME;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
    rotation = rand() % 360;

    range = 0;
    max_range = 150;

    status = ALIVE;
    ttl = 20;
};

Bullet::~Bullet(){ /*cout<<"Deleting Bullet.\n";*/ };

inline void Bullet::Draw1()
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( color1 );
    glBegin( GL_LINES );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it += 3;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glBegin( GL_LINES );
    glColor4f( color1[ 0 ], color1[ 1 ], color1[ 2 ], 1 );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it += 5;
    glColor4f( color1[ 0 ], color1[ 1 ], color1[ 2 ], 0 );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glPopMatrix();
};

inline void Bullet::DrawLaser()
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glBegin( GL_LINES );
    glColor4fv( color1 );
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    it++;
    glVertex3d( ( *it ).x, ( *it ).y, ( *it ).z );
    glEnd();
    glPopMatrix();
}

inline void Bullet::Draw2()
{
    Tail::const_iterator it = m_tail.begin();
    glPushMatrix();
    glColor4fv( color1 );
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

void Bullet::Draw()
{
    if ( status == DEAD ) {
        return;
    }
    switch ( type ) {
    case SLUG:
        DrawLaser();
        break;
    case BLASTER:
        Draw1();
        break;
    case TORPEDO:
        Draw2();
        break;
    case WAVE:
        break;
    case MINE:
        break;
    }
};

void Bullet::Update()
{
    if ( status == DEAD ) {
        return;
    }
    if ( range > max_range ) {
        status = DEAD;
        return;
    }

    switch ( type ) {
    case SLUG:
        color1[ 3 ] -= 2.0 * DELTATIME;
        //         tmp2 = 1-(max_range/range);
        //         if(tmp2>=0){ color1[3]=tmp2; } else { color1[3]=0; }
        range += max_range * 2.0 * DELTATIME;
        break;
    case TORPEDO:
        InterceptTarget(); /*no break*/
    case BLASTER:
        position = position + velocity * DELTATIME;
        m_tail.insert( position );
        range += speed * DELTATIME;
        break;
    case WAVE:
        break;
    case MINE:
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
    const double tmp = dot_product( dir, collRay );
    double dist;

    if ( tmp <= 0 ) {
        dist = length_v( dir );
    }
    else {
        const double tmp2 = dot_product( collRay, collRay );
        if ( tmp2 <= tmp ) {
            dist = length_v( dir );
        }
        else {
            const Vertex Pb = position + ( collRay * ( tmp / tmp2 ) );
            dist = length_v( object->GetPosition() - Pb );
        }
    }

    return dist < ( CollisionDistance + object->GetCollisionDistance() );
}

void Bullet::ProcessCollision( SAObject* object )
{
    assert( object );
    if ( collisionTest( object ) ) {
        object->AddScore( score );
        switch ( type ) {
        case SLUG:
            object->Damage( damage * color1[ 3 ] );
            break;
        default:
            object->Damage( damage );
            status = DEAD;
            break;
        }
    }
}

Vertex Bullet::collisionRay() const
{
    switch ( type ) {
    case BLASTER:
        return direction * speed * DELTATIME * 3;
    case TORPEDO:
        return direction * speed * DELTATIME;
    case SLUG:
        return direction * 1000;
    default:
        assert( !"unreachable" );
        return Vertex();
    }
}

void Bullet::SetDirection( Vertex v )
{
    direction = v;
    normalise_v( direction );
    velocity = direction * speed;
    if ( type == SLUG ) {
        m_tail.insert( position + ( direction * 1000 ) );
    }
}

GLuint Bullet::getDamage()
{
    return damage;
}
//   GLFloat[] getCoords() { return {x,y,z}; }

GLuint Bullet::GetType()
{
    return type;
}
