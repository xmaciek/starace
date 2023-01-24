#include "bullet.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <random>

static constexpr float c_maxRange = 6000.0_m;
static constexpr float c_tailChunkLength = 5.0_m;

Bullet::Bullet( const WeaponCreateInfo& bp, const math::vec3& position )
: m_color1{ bp.color1 }
, m_color2{ bp.color2 }
, m_texture{ bp.texture }
, m_score{ bp.score_per_hit }
, m_damage{ bp.damage }
, m_type{ bp.type }
{
    std::fill( m_tail.begin(), m_tail.end(), position );
    m_speed = bp.speed;
    m_position = position;
    setStatus( Status::eAlive );
};

void Bullet::render( RenderContext ) const
{
    assert( !"deleted function" );
}

void Bullet::renderAll( const RenderContext& rctx, std::span<const UniquePointer<Bullet>> span )
{
    if ( span.empty() ) return;

    using ParticleBlob = PushConstant<Pipeline::eParticleBlob>;
    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eParticleBlob ],
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = span[ 0 ]->m_texture;

    ParticleBlob pushConstant{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_cameraPosition = rctx.cameraPosition,
        .m_cameraUp = rctx.cameraUp,
    };

    auto pushBullet = []( auto& pushConstant, uint32_t idx, const Bullet& bullet )
    {
        assert( ( idx + 5 ) <= ParticleBlob::INSTANCES );

        auto makeParticle = []( const math::vec3& pos, float size, const math::vec4& color ) -> ParticleBlob::Particle
        {
            return {
                .m_position = math::vec4{ pos.x, pos.y, pos.z, size },
                .m_uvxywh = math::makeUVxywh<1, 1>( 0, 0 ),
                .m_color = color,
            };
        };
        const float size = 2.6_m;
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
        assert( it );
        if ( !isOnScreen( mvp, it->m_position, rctx.viewport ) ) {
            continue;
        }
        idx = pushBullet( pushConstant, idx, *it );
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

void Bullet::update( const UpdateContext& updateContext )
{
    assert( status() != Status::eDead );

    if ( m_range > c_maxRange ) {
        setStatus( Status::eDead );
        return;
    }

    switch ( m_type ) {
    case Type::eTorpedo:
        if ( m_target && m_target->status() != Status::eDead ) {
            m_direction = interceptTarget( m_direction, m_position, m_target->position(), 160.0_deg * updateContext.deltaTime );
        }
        [[fallthrough]];

    case Type::eBlaster:
        m_prevPosition = m_position;
        m_position += velocity() * updateContext.deltaTime;
        {
            math::vec3 pos = m_position;
            for ( auto& it : m_tail ) {
                if ( math::distance( it, pos ) > c_tailChunkLength ) {
                    it = pos + math::normalize( it - pos ) * c_tailChunkLength;
                }
                pos = it;
            }
        }
        m_range += m_speed * updateContext.deltaTime;
        break;
    }
};

void Bullet::setDirection( const math::vec3& v )
{
    m_direction = math::normalize( v );
}

uint16_t Bullet::score() const
{
    return m_score;
}

uint8_t Bullet::damage() const
{
    return m_damage;
}

Bullet::Type Bullet::type() const
{
    return m_type;
}

math::vec3 Bullet::prevPosition() const
{
    return m_prevPosition;
}
