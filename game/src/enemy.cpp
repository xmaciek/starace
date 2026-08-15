#include "enemy.hpp"

#include "autoaim.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <profiler.hpp>

#include <cassert>
#include <ranges>

Enemy::Enemy( const CreateInfo& ci )
: m_weapon{ ci.weapon }
, m_model{ *ci.model }
, m_callsign { ci.callsign }
{
    m_position = math::vec3{
        randomRange( -400_m, 400_m ),
        randomRange( -400_m, 400_m ),
        randomRange( -400_m, 400_m )
    };

    m_speed = 900_kmph;
    setStatus( Status::eAlive );
    m_health = 100;
    m_direction = math::vec3( 0.0f, 0.0f, 1.0f );
}

math::quat Enemy::quat() const
{
    return math::quatLookAt( m_direction, { 0.0f, 1.0f, 0.0f } );
}

void Enemy::shoot( std::pmr::vector<Bullet>& vec )
{
    if ( !m_weapon.ready() ) return;
    if ( !AutoAim{}.matches( position(), direction(), m_target.position ) ) return;
    auto& b = vec.emplace_back( m_weapon.fire(), position(), direction() );
    b.m_collideId = COLLIDE_ID;
    b.m_quat = quat();
}

Signal Enemy::signal() const
{
    return Signal{
        .position = position(),
        .team = 0,
        .callsign = m_callsign,
    };
}

void Enemy::renderAll( const RenderContext& rctx, std::span<const Enemy> span )
{
    ZoneScoped;
    auto culling = [&rctx]( const Enemy& e )
    {
        const math::vec3 screenPos = project3dTo2d( rctx.camera3d, e.m_position, rctx.viewport );
        return isOnScreen( screenPos, rctx.viewport );
    };
    auto r = rctx;
    auto render = [&rctx, &r]( const Enemy& e )
    {
        assert( e.status() == Status::eAlive );
        r.model = math::translate( rctx.model, e.m_position ) * math::toMat4( e.quat() );
        e.m_model.render( r );
    };
    std::ranges::for_each( span | std::views::filter( culling ), render );
}

void Enemy::updateAll( const UpdateContext& uctx, std::span<Enemy> enemies )
{
    auto update = [&uctx]( auto& e )
    {
        assert( e.status() != Status::eDead );
        e.m_weapon.update( uctx );
        e.m_direction = interceptTarget( e.m_direction, e.m_position, e.m_target.position, 30.0_deg * uctx.deltaTime );
        e.m_position += e.velocity() * uctx.deltaTime;
        e.m_health -= std::min( e.m_health, e.m_pendingDamage );
        e.m_pendingDamage = 0;
        e.m_status = e.m_health ? e.m_status : Status::eDead;
    };
    std::ranges::for_each( enemies, update );
}

