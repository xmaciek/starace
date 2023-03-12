#include "enemy.hpp"

#include "autoaim.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <Tracy.hpp>

#include <cassert>

Enemy::Enemy( Model* m )
: m_model{ *m }
{
    m_position = math::vec3{
        randomRange( -10.0f, 10.0f ),
        randomRange( -10.0f, 10.0f ),
        randomRange( -10.0f, 10.0f )
    };

    m_speed = 700_kmph;
    setStatus( Status::eAlive );
    m_health = 100;
    m_direction = math::vec3( 0.0f, 0.0f, 1.0f );
}

void Enemy::setWeapon( const WeaponCreateInfo& w )
{
    m_weapon = w;
    m_shotFactor = randomRange( 0, m_weapon.delay );
}

UniquePointer<Bullet> Enemy::weapon( std::pmr::memory_resource* alloc )
{
    assert( alloc );
    UniquePointer<Bullet> bullet{ alloc, m_weapon, position() };
    bullet->setDirection( direction() );
    bullet->setTarget( m_target );
    m_shotFactor = 0;
    return bullet;
}


bool Enemy::isWeaponReady() const
{
    if ( m_shotFactor < m_weapon.delay ) {
        return false;
    }
    return AutoAim{}.matches( position(), direction(), m_target->position() );
}


void Enemy::renderAll( const RenderContext& rctx, std::span<const UniquePointer<Enemy>> span )
{
    ZoneScoped;
    auto r = rctx;
    for ( const auto& ptr : span ) {
        assert( ptr->status() == Status::eAlive );
        const math::vec3 screenPos = project3dTo2d( rctx.camera3d, ptr->m_position, rctx.viewport );
        if ( !isOnScreen( screenPos, rctx.viewport ) ) { continue; }

        const math::quat quat = math::quatLookAt( ptr->m_direction, { 0.0f, 1.0f, 0.0f } );
        r.model = math::translate( rctx.model, ptr->m_position ) * math::toMat4( quat );
        ptr->m_model.render( r );
        for ( auto it : ptr->m_model.thrusters() ) {
            ptr->m_thruster.renderAt( r, it );
        }
    }
}

void Enemy::render( RenderContext ) const
{
    assert( !"not callable" );
}

void Enemy::update( const UpdateContext& updateContext )
{
    assert( status() != Status::eDead );
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }

    m_shotFactor = std::min( m_weapon.delay, m_shotFactor + updateContext.deltaTime );
    m_direction = interceptTarget( m_direction, m_position, m_target->position(), 30.0_deg * updateContext.deltaTime );
    m_position += velocity() * updateContext.deltaTime;
}

