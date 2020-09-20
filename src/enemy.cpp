#include "enemy.hpp"

#include <cassert>

Enemy::Enemy()
: m_shield( 0.1, 0.02 )
{
    reinitCoordinates();
    m_speed = 2.1;
    setStatus( Status::eAlive );
    m_health = 100;
    m_direction.z = 1;

    m_collisionDistance = 0.1;
    m_collisionFlag = true;

    normalizeV( m_direction );
    m_velocity = m_direction * speed();
    m_turnrate = speed() * 5 * DEG2RAD * DELTATIME;
    m_ttl = 10;
}

void Enemy::setWeapon( const BulletProto& b )
{
    m_weapon = b;
    m_shotFactor = randomRange( 0, m_weapon.delay );
}

Bullet* Enemy::weapon()
{
    m_weapon.x = m_position.x;
    m_weapon.y = m_position.y;
    m_weapon.z = m_position.z;
    Bullet* bullet = new Bullet( m_weapon );
    bullet->setDirection( direction() );
    bullet->setTarget( m_target );
    m_shotFactor = 0;
    return bullet;
}

bool Enemy::isWeaponReady() const
{
    return m_shotFactor >= m_weapon.delay;
}

void Enemy::reinitCoordinates()
{
    m_position.x = randomRange( -10.0, 10.0 );
    m_position.y = randomRange( -10.0, 10.0 );
    m_position.z = randomRange( -10.0, 10.0 );
}

void Enemy::draw() const
{
    if ( status() != Status::eAlive ) {
        return;
    }
    glPushMatrix();
    glTranslated( m_position.x, m_position.y, m_position.z );
    glColor3f(
        1.0f - m_healthPerc + colorHalf( 1.0f - m_healthPerc )
        , colorHalf( m_healthPerc ) + m_healthPerc
        , 0 );
    m_shield.draw();
    if ( m_isTargeted ) {
        drawCollisionIndicator();
    }
    glPopMatrix();
}

void Enemy::update()
{
    if ( status() != Status::eAlive ) {
        return;
    }
    m_shield.update();
    if ( m_shotFactor < m_weapon.delay ) {
        m_shotFactor += 1.0 * DELTATIME;
    }
    interceptTarget();
    m_position += velocity() * DELTATIME;
    m_healthPerc = m_health / 100;
}

void Enemy::drawCollisionIndicator()
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

void Enemy::drawRadarPosition( const Vertex& modifier, GLdouble scale ) const
{
    if ( status() != Status::eAlive ) {
        return;
    }
    Vertex radarPosition = modifier;
    radarPosition = ( position() - modifier ) * ( scale / 25 );
    if ( lengthV( radarPosition ) > scale ) {
        normalizeV( radarPosition );
        radarPosition = radarPosition * scale;
        glColor3f( 1, 0.4, 0.05 );
    }
    else {
        glColor4f( 1, 1, 1, 0.9 );
    }
    glPushMatrix();
    glBegin( GL_LINES );
    glVertex3d( radarPosition.x, radarPosition.y, radarPosition.z );
    glVertex3d( 0, 0, 0 );
    glEnd();

    glPopMatrix();
}

void Enemy::processCollision( SAObject* object )
{
    assert( object );
    if ( !object->canCollide() || status() == Status::eDead || object->status() != Status::eAlive ) {
        return;
    }

    if ( distanceV( position(), object->position() ) <= collisionDistance() + object->collisionDistance() ) {
        setDamage( collisionDamage() + object->collisionDamage() );
        object->setDamage( collisionDamage() + object->collisionDamage() );
    }
}
