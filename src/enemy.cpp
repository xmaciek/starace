#include "enemy.hpp"

#include "utils.hpp"

#include <cassert>

Enemy::Enemy( Model* m )
: m_model{ m }
{
    m_position = math::vec3{
        randomRange( -10.0f, 10.0f ),
        randomRange( -10.0f, 10.0f ),
        randomRange( -10.0f, 10.0f )
    };

    m_speed = 4.2f;
    setStatus( Status::eAlive );
    m_health = 100;
    m_direction = math::vec3( 0.0f, 0.0f, 1.0f );

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

void Enemy::render( RenderContext rctx ) const
{
    assert( status() == Status::eAlive );
    if ( !m_isOnScreen ) {
        return;
    }

    math::quat quat = math::quatLookAt( m_direction, { 0.0f, 1.0f, 0.0f } );
    rctx.model = math::translate( rctx.model, position() );
    rctx.model = rctx.model * math::toMat4( quat );
    m_model->render( rctx );
    for ( auto it : m_model->thrusters() ) {
        m_thruster.renderAt( rctx, it );
    }
}

void Enemy::update( const UpdateContext& updateContext )
{
    assert( status() != Status::eDead );
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }

    m_shotFactor = std::min( m_weapon.delay, m_shotFactor + updateContext.deltaTime );
    m_healthPerc = (float)m_health / 100;
    m_turnrate = math::radians( speed() * 5 * updateContext.deltaTime );
    interceptTarget();
    m_position += velocity() * updateContext.deltaTime;
    m_screenPos = project3dTo2d( updateContext.camera, m_position, updateContext.viewport );
    m_isOnScreen = isOnScreen( m_screenPos, updateContext.viewport );
}

