#include "bullet.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>

Bullet::Bullet( const WeaponCreateInfo& bp, const math::vec3& position, const math::vec3& direction )
: m_position{ position }
, m_direction{ direction }
, m_prevPosition{ position }
, m_target{ bp.target }
, m_mesh{ bp.mesh }
, m_texture{ bp.texture }
, m_speed{ bp.speed }
, m_maxDistance{ bp.distance }
, m_score{ bp.score_per_hit }
, m_damage{ bp.damage }
, m_type{ bp.type }
{
};

void Bullet::renderAll( const RenderContext& rctx, std::span<Bullet> span, Texture )
{
    if ( span.empty() ) return;

    std::sort( span.begin(), span.end(), []( const auto& lhs, const auto& rhs ) {
        if ( lhs.m_mesh != rhs.m_mesh ) return lhs.m_mesh < rhs.m_mesh;
        return lhs.m_texture < rhs.m_texture;
    } );
    using PushConstant = PushConstant<Pipeline::eProjectile>;
    PushConstant pc{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    PushData pd{
        .m_pipeline = g_pipelines[ Pipeline::eProjectile ],
    };

    const math::mat4 mvp = rctx.projection * rctx.view * rctx.model;
    Buffer lastMesh{};
    Texture lastTexture{};
    uint32_t idx = 0;

    for ( auto&& bullet : span ) {
        if ( bullet.m_type == Type::eLaser ) continue;
        if ( !isOnScreen( mvp, bullet.m_position, rctx.viewport ) ) continue;
        assert( bullet.m_mesh );
        assert( bullet.m_texture );
        if ( lastMesh != bullet.m_mesh || lastTexture != bullet.m_texture || idx == PushConstant::INSTANCES ) {
            pd.m_fragmentTexture[ 1 ] = lastTexture;
            pd.m_vertexBuffer = lastMesh;
            pd.m_instanceCount = idx;
            if ( lastMesh && lastTexture ) {
                rctx.renderer->push( pd, &pc );
            }
            idx = 0;
        }
        lastMesh = bullet.m_mesh;
        lastTexture = bullet.m_texture;
        pc.m_projectiles[ idx++ ] = PushConstant::Projectile{ bullet.m_quat, math::vec4{ bullet.m_position, meter } };
    }
    if ( idx != 0 ) {
        pd.m_fragmentTexture[ 1 ] = lastTexture;
        pd.m_vertexBuffer = lastMesh;
        pd.m_instanceCount = idx;
        rctx.renderer->push( pd, &pc );
    }
}

void Bullet::updateAll( const UpdateContext& uctx, std::span<Bullet> span, std::pmr::vector<Explosion>& explosions, Texture texture )
{
    auto update = [&uctx, &explosions, texture]( auto& bullet )
    {
        switch ( bullet.m_type ) {
        case Type::eTorpedo:
            if ( bullet.m_target ) {
                Signal ret = bullet.m_target;
                auto evalSignal = [&ret, pos = ret.position, dist = std::numeric_limits<float>::max(), team = bullet.m_collideId]( const Signal& sig ) mutable
                {
                    if ( sig.team == team ) return;
                    float d = math::manhattan( pos, sig.position );
                    if ( d > dist ) return;
                    ret = sig;
                    dist = d;
                };
                std::for_each( uctx.signals.begin(), uctx.signals.end(), std::move( evalSignal ) );
                bullet.m_target = ret;
                const math::vec3 tgtDir = math::normalize( bullet.m_target.position - bullet.m_position );
                const float angle = math::angle( bullet.m_direction, tgtDir );
                const float anglePerUpdate = std::min( angle, 160.0_deg * uctx.deltaTime );
                bullet.m_direction = math::normalize( math::slerp( bullet.m_direction, tgtDir, anglePerUpdate / angle ) );
            }
            bullet.m_quat = math::quatLookAt( bullet.m_direction, { 0.0f, 1.0f, 0.0f } );
            // rocket trail
            explosions.emplace_back() = Explosion{
                .m_position = bullet.m_position,
                .m_velocity = -bullet.m_direction * bullet.m_speed * 0.1f,
                .m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                .m_texture = texture,
                .m_size = 2.0_m,
                .m_duration = 1.5f,
            };
            [[fallthrough]];

        case Type::eLaser:
        case Type::eBlaster:
            bullet.m_prevPosition = bullet.m_position;
            bullet.m_position += bullet.m_direction * bullet.m_speed * uctx.deltaTime;
            bullet.m_travelDistance += bullet.m_speed * uctx.deltaTime;
            break;

        case Type::eDead:
            break;
        }
        if ( bullet.m_travelDistance >= bullet.m_maxDistance ) {
            bullet.m_type = Bullet::Type::eDead;
        }
    };
    std::for_each( span.begin(), span.end(), update );
};



Weapon::Weapon( const WeaponCreateInfo& ci )
: m_ci{ ci }
, m_reload{ ci.reload }
, m_count{ ci.capacity }
{
}

void Weapon::update( const UpdateContext& uctx )
{
    m_delay = std::min( m_delay + uctx.deltaTime, m_ci.delay );
    if ( m_count >= m_ci.capacity ) return;
    m_reload += uctx.deltaTime;
    if ( m_reload < m_ci.reload ) return;
    m_count++;
    if ( m_count < m_ci.capacity ) m_reload -= m_ci.reload;
}

bool Weapon::ready() const
{
    return m_count > 0 && m_delay >= m_ci.delay;
}

WeaponCreateInfo Weapon::fire()
{
    m_count--;
    m_delay = 0.0f;
    return m_ci;
}
