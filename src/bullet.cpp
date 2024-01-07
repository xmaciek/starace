#include "bullet.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <random>

static constexpr float TAIL_CHUNK_LENGTH = 5.0_m;

Bullet::Bullet( const WeaponCreateInfo& bp, const math::vec3& position, const math::vec3& direction )
: m_position{ position }
, m_direction{ direction }
, m_color1{ bp.color1 }
, m_color2{ bp.color2 }
, m_speed{ bp.speed }
, m_maxDistance{ bp.distance }
, m_size{ bp.size }
, m_score{ bp.score_per_hit }
, m_damage{ bp.damage }
, m_type{ bp.type }
{
    std::fill( m_tail.begin(), m_tail.end(), position );
};

void Bullet::renderAll( const RenderContext& rctx, std::span<Bullet> span, Texture texture )
{
    if ( span.empty() ) return;

    using ParticleBlob = PushConstant<Pipeline::eParticleBlob>;
    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eParticleBlob ],
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = texture;

    ParticleBlob pushConstant{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_cameraPosition = rctx.cameraPosition,
        .m_cameraUp = rctx.cameraUp,
    };

    auto pushBullet = []( auto& pushConstant, uint32_t idx, const Bullet& bullet )
    {
        assert( ( idx + 5 ) <= ParticleBlob::INSTANCES );
        assert( bullet.m_type != Type::eDead );

        auto makeParticle = []( const math::vec3& pos, float size, const math::vec4& color ) -> ParticleBlob::Particle
        {
            return {
                .m_position = math::vec4{ pos.x, pos.y, pos.z, size },
                .m_uvxywh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f },
                .m_color = color,
            };
        };
        const float size = bullet.m_size;
        pushConstant.m_particles[ idx++ ] = makeParticle( bullet.m_position, size, bullet.m_color1 );
        pushConstant.m_particles[ idx++ ] = makeParticle( bullet.m_tail[ 0 ], size * 0.8f, bullet.m_color2 );
        pushConstant.m_particles[ idx++ ] = makeParticle( bullet.m_tail[ 1 ], size * 0.6f, bullet.m_color2 );
        pushConstant.m_particles[ idx++ ] = makeParticle( bullet.m_tail[ 2 ], size * 0.4f, bullet.m_color2 );
        pushConstant.m_particles[ idx++ ] = makeParticle( bullet.m_tail[ 3 ], size * 0.2f, bullet.m_color2 );
        return idx;
    };

    const math::mat4 mvp = rctx.projection * rctx.view * rctx.model;

    uint32_t idx = 0;
    for ( const auto& it : span ) {
        if ( it.m_type == Type::eLaser ) continue;
        if ( !isOnScreen( mvp, it.m_position, rctx.viewport ) ) continue;
        idx = pushBullet( pushConstant, idx, it );
        if ( ParticleBlob::INSTANCES - idx < 5 ) {
            pushBuffer.m_instanceCount = idx;
            rctx.renderer->push( pushBuffer, &pushConstant );
            idx = 0;
        }
    }
    if ( idx != 0 ) {
        pushBuffer.m_instanceCount = idx;
        rctx.renderer->push( pushBuffer, &pushConstant );
    }
}

void Bullet::updateAll( const UpdateContext& updateContext, std::span<Bullet> span )
{
    auto update = [dt = updateContext.deltaTime]( auto& bullet )
    {
        switch ( bullet.m_type ) {
        case Type::eTorpedo:
            if ( bullet.m_target ) {
                bullet.m_direction = interceptTarget( bullet.m_direction, bullet.m_position, bullet.m_target->position(), 160.0_deg * dt );
            }
            [[fallthrough]];

        case Type::eLaser:
        case Type::eBlaster:
            bullet.m_prevPosition = bullet.m_position;
            bullet.m_position += bullet.m_direction * bullet.m_speed * dt;
            bullet.m_travelDistance += bullet.m_speed * dt;
            if ( bullet.m_travelDistance >= TAIL_CHUNK_LENGTH ) [[likely]]
            {
                math::vec3 pos = bullet.m_position;
                for ( auto& it : bullet.m_tail ) {
                    it = pos + math::normalize( it - pos ) * TAIL_CHUNK_LENGTH;
                    pos = it;
                }
            }
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
