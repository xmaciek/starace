#include "bullet.hpp"

#include "constants.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <random>

static constexpr float c_maxRange = 6000.0_m;
static constexpr float c_tailChunkLength = 5.0_m;

Bullet::Bullet( const BulletProto& bp )
: m_color1{ bp.color1 }
, m_color2{ bp.color2 }
, m_texture{ bp.texture }
, m_score{ bp.score_per_hit }
, m_damage{ bp.damage }
, m_type{ bp.type }
{
    std::fill( m_tail.begin(), m_tail.end(), bp.position );
    m_speed = bp.speed;
    m_position = bp.position;
    setStatus( Status::eAlive );
};

void Bullet::render( RenderContext rctx ) const
{
    assert( status() == Status::eAlive );
    // culling
    {
        const math::mat4 mvp = rctx.projection * rctx.view * rctx.model;
        const math::vec2 viewport = rctx.viewport;
        if ( !isOnScreen( mvp, m_position, viewport ) ) {
            return;
        }
    }

    const float size = 2.6_m;
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eSprite3D ),
        .m_verticeCount = 4,
        .m_texture = m_texture,
    };

    PushConstant<Pipeline::eSprite3D> plasmaFace{
        .m_model = math::billboard( m_position, rctx.cameraPosition, rctx.cameraUp ),
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color1,
        .m_vertices = {
            math::vec4{ -size, -size, 0, 0 },
            math::vec4{ -size, size, 0, 0 },
            math::vec4{ size, size, 0, 0 },
            math::vec4{ size, -size, 0, 0 },
        },
        .m_uv = {
            math::vec4{ 0, 0, 0, 0 },
            math::vec4{ 0, 1, 0, 0 },
            math::vec4{ 1, 1, 0, 0 },
            math::vec4{ 1, 0, 0, 0 },
        },
    };
    rctx.renderer->push( pushBuffer, &plasmaFace );

    pushBuffer.m_pipeline = static_cast<PipelineSlot>( Pipeline::eThruster );
    pushBuffer.m_texture = {};

    PushConstant<Pipeline::eThruster> afterGlow{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color2,
        .m_xyuv = {
            math::vec4{ -size, -size, 0, 0 },
            math::vec4{ -size, size, 0, 1 },
            math::vec4{ size, size, 1, 1 },
            math::vec4{ size, -size, 1, 0 },
        },
    };
    const float r[ 4 ] = { 0.15f, 0.2f, 0.25f, 0.3f };
    const float distance = math::distance( m_position, rctx.cameraPosition );
    const float lodRange[] = { 1000.0_m, 800.0_m, 600.0_m, 400.0_m };
    for ( uint32_t i : { 0u, 1u, 2u, 3u } ) {
        if ( distance > lodRange[ i ] ) { continue; }
        afterGlow.m_radius = r[ i ];
        afterGlow.m_model = math::billboard( m_tail[ i ], rctx.cameraPosition, rctx.cameraUp );
        rctx.renderer->push( pushBuffer, &afterGlow );
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
