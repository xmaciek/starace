#include "enemy.hpp"

#include "autoaim.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <profiler.hpp>

#include <cassert>

Enemy::Enemy( const CreateInfo& ci )
: m_weapon{ ci.weapon }
, m_model{ *ci.model }
, m_callsign { ci.callsign }
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

math::quat Enemy::quat() const
{
    return math::quatLookAt( m_direction, { 0.0f, 1.0f, 0.0f } );
}

void Enemy::shoot( std::pmr::vector<Bullet>& vec )
{
    if ( !m_weapon.ready() ) return;
    if ( !AutoAim{}.matches( position(), direction(), m_target->position() ) ) return;
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

void Enemy::renderAll( const RenderContext& rctx, std::span<const UniquePointer<Enemy>> span )
{
    ZoneScoped;
    auto r = rctx;
    for ( const auto& ptr : span ) {
        assert( ptr->status() == Status::eAlive );
        const math::vec3 screenPos = project3dTo2d( rctx.camera3d, ptr->m_position, rctx.viewport );
        if ( !isOnScreen( screenPos, rctx.viewport ) ) { continue; }

        r.model = math::translate( rctx.model, ptr->m_position ) * math::toMat4( ptr->quat() );
        ptr->m_model.render( r );
    }
}

void Enemy::updateAll( const UpdateContext& uctx, std::span<UniquePointer<Enemy>> e )
{
    auto update = [&uctx]( auto& p )
    {
        assert( p->status() != Status::eDead );
        p->m_weapon.update( uctx );
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

