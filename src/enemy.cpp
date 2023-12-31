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
}

void Enemy::shoot( std::pmr::vector<Bullet>& vec )
{
    if ( m_shotFactor < m_weapon.delay ) {
        return;
    }
    if ( !AutoAim{}.matches( position(), direction(), m_target->position() ) ) {
        return;
    }
    m_shotFactor = 0;
    auto& b = vec.emplace_back( m_weapon, position(), direction() );
    b.m_collideId = COLLIDE_ID;
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

void Enemy::update( const UpdateContext& )
{
    assert( !"not callable" );
}

void Enemy::updateAll( const UpdateContext& uctx, std::span<UniquePointer<Enemy>> e )
{
    auto update = [&uctx]( auto& p )
    {
        assert( p->status() != Status::eDead );
        p->m_shotFactor = std::min( p->m_weapon.delay, p->m_shotFactor + uctx.deltaTime );
        p->m_direction = interceptTarget( p->m_direction, p->m_position, p->m_target->position(), 30.0_deg * uctx.deltaTime );
        p->m_position += p->velocity() * uctx.deltaTime;
        p->m_health -= std::min( p->m_health, p->m_pendingDamage );
        p->m_pendingDamage = 0;
        if ( p->m_health == 0 ) {
            p->m_status = Status::eDead;
        }
    };
    std::for_each( e.begin(), e.end(), update );
}

