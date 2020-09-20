#include "enemy.hpp"

#include <cassert>

Enemy::Enemy()
: m_shield( 0.1, 0.02 )
, m_outRange{ 1.0 }
, m_healthPerc{ 1.0f }
{
    ReinitCoordinates();
    speed = 2.1;
    status = ALIVE;
    health = 100;
    direction.z = 1;

    score = 0;

    CollisionDistance = 0.1;
    CollisionFlag = true;

    normalise_v( direction );
    velocity = direction * speed;
    turnrate_in_rads = speed * 5 * DEG2RAD * DELTATIME;
    ImTargeted = false;
    ttl = 10;
}

void Enemy::SetWeapon( const BulletProto& b )
{
    m_weapon = b;
    m_shotFactor = random_range( 0, m_weapon.delay );
}

Bullet* Enemy::GetWeapon()
{
    m_weapon.x = position.x;
    m_weapon.y = position.y;
    m_weapon.z = position.z;
    Bullet* bullet = new Bullet( m_weapon );
    bullet->SetDirection( direction );
    bullet->SetTarget( target );
    m_shotFactor = 0;
    return bullet;
}

bool Enemy::IsWeaponReady() const
{
    return m_shotFactor >= m_weapon.delay;
}

void Enemy::ReinitCoordinates()
{
    position.x = random_range( -10.0, 10.0 );
    position.y = random_range( -10.0, 10.0 );
    position.z = random_range( -10.0, 10.0 );
}

void Enemy::Draw() const
{
    if ( status == ALIVE ) {
        glPushMatrix();
        glTranslated( position.x, position.y, position.z );
        glColor3f(
            1.0f - m_healthPerc + colorhalf( 1.0f - m_healthPerc )
            , colorhalf( m_healthPerc ) + m_healthPerc
            , 0 );
        m_shield.draw();
        if ( ImTargeted ) {
            DrawCollisionIndicator();
        }
        glPopMatrix();
    }
}

void Enemy::Update()
{
    if ( status == ALIVE ) {
        m_shield.update();
        if ( m_shotFactor < m_weapon.delay ) {
            m_shotFactor += 1.0 * DELTATIME;
        }
        InterceptTarget();
        position = position + velocity * DELTATIME;
        m_healthPerc = health / 100;
    }
}

void Enemy::DrawCollisionIndicator()
{
    glColor3f( 1, 0.1, 0.1 );
    glLineWidth( 2 );
    glBegin( GL_LINE_LOOP );
    glVertex2d( -0.125, 0.125 );
    glVertex2d( -0.125, -0.125 );
    glVertex2d( 0.125, -0.125 );
    glVertex2d( 0.125, 0.125 );
    glEnd();
    glLineWidth( 1 );
}

void Enemy::DrawRadarPosition( const Vertex& Modifier, const GLdouble& RadarScale ) const
{
    if ( status != ALIVE ) {
        return;
    }
    Vertex RadarPosition = Modifier;
    RadarPosition = ( position - Modifier ) * ( RadarScale / 25 );
    if ( length_v( RadarPosition ) > RadarScale ) {
        normalise_v( RadarPosition );
        RadarPosition = RadarPosition * RadarScale;
        glColor3f( 1, 0.4, 0.05 );
    }
    else {
        glColor4f( 1, 1, 1, 0.9 );
    }
    glPushMatrix();
    glBegin( GL_LINES );
    glVertex3d( RadarPosition.x, RadarPosition.y, RadarPosition.z );
    glVertex3d( 0, 0, 0 );
    glEnd();

    glPopMatrix();
}

void Enemy::ProcessCollision( SAObject* object )
{
    assert( object );
    if ( !object->CanCollide() || status == DEAD || object->GetStatus() != ALIVE ) {
        return;
    }

    if ( distance_v( position, object->GetPosition() ) <= CollisionDistance + object->GetCollisionDistance() ) {
        Damage( CollisionDamage + object->GetCollisionDamage() );
        object->Damage( CollisionDamage + object->GetCollisionDamage() );
    }
}
