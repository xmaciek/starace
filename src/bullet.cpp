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
, m_color1{ bp.color1 }
, m_color2{ bp.color2 }
, m_target{ bp.target }
, m_speed{ bp.speed }
, m_maxDistance{ bp.distance }
, m_size{ bp.size }
, m_score{ bp.score_per_hit }
, m_damage{ bp.damage }
, m_type{ bp.type }
{
};

void Bullet::renderAll( const RenderContext& rctx, std::span<Bullet> span, Texture )
{
    if ( span.empty() ) return;

    PushData bd{
        .m_pipeline = g_pipelines[ Pipeline::eBeamBlob ],
        .m_verticeCount = 12,
    };
    using PushConstant = PushConstant<Pipeline::eBeamBlob>;
    PushConstant bc{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    const math::mat4 mvp = rctx.projection * rctx.view * rctx.model;

    uint32_t idx = 0;
    for ( auto&& bullet : span ) {
        if ( bullet.m_type == Type::eLaser ) continue;
        if ( !isOnScreen( mvp, bullet.m_position, rctx.viewport ) ) continue;
        bc.m_beams[ idx++ ] = PushConstant::Beam{
            .m_position = bullet.m_position,
            .m_quat = bullet.m_quat,
            .m_displacement{ bullet.m_size, bullet.m_size, bullet.m_speed * 0.01618f },
            .m_color1 = bullet.m_color1,
            .m_color2 = bullet.m_color2,
        };
        if ( idx == PushConstant::INSTANCES ) {
            bd.m_instanceCount = idx;
            rctx.renderer->push( bd, &bc );
            idx = 0;
        }
    }
    if ( idx != 0 ) {
        bd.m_instanceCount = idx;
        rctx.renderer->push( bd, &bc );
    }
}

void Bullet::updateAll( const UpdateContext& updateContext, std::span<Bullet> span, std::pmr::vector<Explosion>& explosions, Texture texture )
{
    auto update = [dt = updateContext.deltaTime, &explosions, texture]( auto& bullet )
    {
        switch ( bullet.m_type ) {
        case Type::eTorpedo:
            if ( bullet.m_target ) {
                const math::vec3 tgtDir = math::normalize( bullet.m_target.position - bullet.m_position );
                const float angle = math::angle( bullet.m_direction, tgtDir );
                const float anglePerUpdate = std::min( angle, 160.0_deg * dt );
                bullet.m_direction = math::normalize( math::slerp( bullet.m_direction, tgtDir, anglePerUpdate / angle ) );
            }
            bullet.m_quat = math::quatLookAt( bullet.m_direction, { 0.0f, 1.0f, 0.0f } );
            explosions.emplace_back() = Explosion{
                .m_position = bullet.m_position,
                .m_velocity = -bullet.m_direction * bullet.m_speed * 0.1f,
                .m_color = bullet.m_color1,
                .m_texture = texture,
                .m_size = 2.0_m,
            };
            [[fallthrough]];

        case Type::eLaser:
        case Type::eBlaster:
            bullet.m_prevPosition = bullet.m_position;
            bullet.m_position += bullet.m_direction * bullet.m_speed * dt;
            bullet.m_travelDistance += bullet.m_speed * dt;
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

void Bullet::scanSignals( std::span<Bullet> bullets, std::span<const Signal> signals )
{
    if ( signals.empty() ) return;
    for ( auto&& bullet : bullets ) {
        if ( bullet.m_type != Type::eTorpedo ) continue;
        if ( !bullet.m_target ) continue;
        Signal ret = bullet.m_target;
        auto evalSignal = [&ret, pos = ret.position, dist = std::numeric_limits<float>::max(), team = bullet.m_collideId]( const Signal& sig ) mutable
        {
            if ( sig.team == team ) return;
            float d = math::manhattan( pos, sig.position );
            if ( d > dist ) return;
            ret = sig;
            dist = d;
        };
        std::for_each( signals.begin(), signals.end(), std::move( evalSignal ) );
        bullet.m_target = ret;
    }
}
