#include "enemy.hpp"

#include "utils.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>

Enemy::Enemy()
: m_shield( 0.1, 0.02 )
{
    reinitCoordinates();
    m_speed = 2.1f;
    setStatus( Status::eAlive );
    m_health = 100;
    m_direction = glm::vec3( 0.0f, 0.0f, 1.0f );

    m_collisionDistance = 0.1;
    m_collisionFlag = true;

    m_velocity = m_direction * speed();
}

void Enemy::setWeapon( const BulletProto& b )
{
    m_weapon = b;
    m_shotFactor = randomRange( 0, m_weapon.delay );
}

Bullet* Enemy::weapon( void* ptr )
{
    assert( ptr );
    m_weapon.position = m_position;
    Bullet* bullet = new ( ptr ) Bullet( m_weapon );
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
    m_position = glm::vec3(
        randomRange( -10.0, 10.0 ), randomRange( -10.0, 10.0 ), randomRange( -10.0, 10.0 ) );
}

void Enemy::render( RenderContext rctx ) const
{
    if ( status() != Status::eAlive ) {
        return;
    }
    rctx.model = glm::translate( rctx.model, position() );
    m_shield.render( rctx );
}

void Enemy::update( const UpdateContext& updateContext )
{
    if ( status() != Status::eAlive ) {
        return;
    }
    m_shield.update( updateContext );
    m_shield.setColor( glm::vec4{
        1.0f - m_healthPerc + colorHalf( 1.0 - m_healthPerc )
        , colorHalf( m_healthPerc ) + m_healthPerc
        , 0.0f
        , 1.0f
    } );
    if ( m_shotFactor < m_weapon.delay ) {
        m_shotFactor += updateContext.deltaTime;
    }

    m_turnrate = glm::radians( speed() * 5 * updateContext.deltaTime );
    interceptTarget();
    m_position += velocity() * updateContext.deltaTime;
    m_healthPerc = (float)m_health / 100;
}

void Enemy::drawCollisionIndicator()
{
//     glColor3f( 1, 0.1, 0.1 );
//     glLineWidth( 2 );
//     glBegin( GL_LINE_LOOP );
//     glVertex2d( -0.125, 0.125 );
//     glVertex2d( -0.125, -0.125 );
//     glVertex2d( 0.125, -0.125 );
//     glVertex2d( 0.125, 0.125 );
//     glEnd();
//     glLineWidth( 1 );
}

void Enemy::drawRadarPosition( const glm::vec3&, float ) const
{
//     if ( status() != Status::eAlive ) {
//         return;
//     }
//     glm::vec3 radarPosition = modifier;
//     radarPosition = ( position() - modifier ) * ( scale / 25 );
//     if ( glm::length( radarPosition ) > scale ) {
//         radarPosition = glm::normalize( radarPosition );
//         radarPosition = radarPosition * scale;
//         glColor3f( 1, 0.4, 0.05 );
//     }
//     else {
//         glColor4f( 1, 1, 1, 0.9 );
//     }
//     glPushMatrix();
//     glBegin( GL_LINES );
//     glVertex3f( radarPosition.x, radarPosition.y, radarPosition.z );
//     glVertex3f( 0, 0, 0 );
//     glEnd();
// 
//     glPopMatrix();
}

