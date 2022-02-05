#include "bullet.hpp"

#include "constants.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <random>

static constexpr uint16_t typeToSegments( Bullet::Type e ) noexcept
{
    switch ( e ) {
    case Bullet::Type::eTorpedo:
        return 24;
    case Bullet::Type::eBlaster:
        return 9;
    case Bullet::Type::eSlug:
        return 2;
    default:
        return 0;
    }
}
static std::array<math::vec4, 23> getTailColors() noexcept
{
    std::array<math::vec4, 23> ret{};
    const math::vec4 a{ 1.0f, 1.0f, 1.0f, 1.0f };
    const math::vec4 diff{ 0.0f, 0.0f, 0.0f, -1.0f };
    size_t i = 0;
    const float f = 1.0f / ret.size();
    for ( auto& it : ret ) {
        const float n = f * (float)i++;
        it = a + diff * ( 0.5f - std::cos( n * constants::pi ) * 0.5f );
    }
    return ret;
}

Bullet::Bullet( const BulletProto& bp )
: m_tail( typeToSegments( bp.type ), bp.position )
, m_color1{ bp.color1 }
, m_color2{ bp.color2 }
, m_texture{ bp.texture }
, m_score{ bp.score_per_hit }
{
    m_type = bp.type;
    m_speed = bp.speed;
    m_damage = bp.damage;
    m_tailChunkLength = m_speed / 60.0f;

    if ( m_type == Type::eSlug ) {
        m_color1 = m_color2;
        m_color1.a = 1.0f;
        m_color2.a = 0.0f;
    }
    m_position = bp.position;
    setStatus( Status::eAlive );
};

void Bullet::render( RenderContext rctx ) const
{
    if ( status() != Status::eAlive ) {
        return;
    }

    assert( m_tail.size() == typeToSegments( m_type ) );

    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dStripColor ),
        .m_lineWidth = 2.0f,
    };

    PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    // culling
    {
        const auto begin = m_tail.begin();
        const auto end = begin + typeToSegments( m_type );
        const math::mat4 mvp = rctx.projection * rctx.view * rctx.model;
        const math::vec2 viewport = rctx.viewport;
        const auto onScreen = [ mvp, viewport ] ( const math::vec3& v ) {
            return isOnScreen( mvp, v, viewport );
        };
        if ( std::none_of( begin, end, onScreen ) ) {
            return;
        }
    }

    switch ( m_type ) {
    case Type::eSlug: {
        pushBuffer.m_verticeCount = 2;
        pushConstant.m_vertices[ 0 ] = { m_tail[ 0 ], 0.0f };
        pushConstant.m_vertices[ 1 ] = { m_tail[ 1 ], 0.0f };
        pushConstant.m_colors[ 0 ] = m_color1;
        pushConstant.m_colors[ 1 ] = m_color1;
    } break;

    case Type::eBlaster: {
        pushBuffer.m_verticeCount = 3;
        pushConstant.m_vertices[ 0 ] = { m_tail[ 0 ], 0.0f };
        pushConstant.m_vertices[ 1 ] = { m_tail[ 3 ], 0.0f };
        pushConstant.m_vertices[ 2 ] = { m_tail[ 8 ], 0.0f };
        pushConstant.m_colors[ 0 ] = m_color1;
        pushConstant.m_colors[ 1 ] = m_color1;
        pushConstant.m_colors[ 2 ] = m_color1;
        pushConstant.m_colors[ 2 ].a = 0.0f;
    } break;

    case Type::eTorpedo: {
        pushBuffer.m_verticeCount = std::min( pushConstant.m_vertices.size(), m_tail.size() );
        std::transform( m_tail.cbegin(), m_tail.cend(), pushConstant.m_vertices.begin(),
            []( math::vec3 v ) { return math::vec4{ v, 0.0f }; }
        );
        pushConstant.m_colors[ 0 ] = m_color1;
        static const std::array<math::vec4, 23> tail = getTailColors();
        std::copy( tail.begin(), tail.end(), pushConstant.m_colors.begin() + 1 );
    } break;
    }

    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_verticeCount = 4;
    pushBuffer.m_pipeline = static_cast<PipelineSlot>( Pipeline::eSprite3D );
    pushBuffer.m_texture = m_texture;
    PushConstant<Pipeline::eSprite3D> pushConstant2{};

    pushConstant2.m_model = math::billboard( m_position, rctx.cameraPosition, rctx.cameraUp );
    pushConstant2.m_view = rctx.view;
    pushConstant2.m_projection = rctx.projection;
    pushConstant2.m_color = m_color1;
    const float size = 2.6_m;
    pushConstant2.m_vertices[ 0 ] = { -size, -size, 0, 0 };
    pushConstant2.m_vertices[ 1 ] = { -size, size, 0, 0 };
    pushConstant2.m_vertices[ 2 ] = { size, size, 0, 0 };
    pushConstant2.m_vertices[ 3 ] = { size, -size, 0, 0 };
    pushConstant2.m_uv[ 0 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant2.m_uv[ 1 ] = { 0.0f, 1.0f, 0.0f, 0.0f };
    pushConstant2.m_uv[ 2 ] = { 1.0f, 1.0f, 0.0f, 0.0f };
    pushConstant2.m_uv[ 3 ] = { 1.0f, 0.0f, 0.0f, 0.0f };
    rctx.renderer->push( pushBuffer, &pushConstant2 );
}

void Bullet::update( const UpdateContext& updateContext )
{
    if ( status() == Status::eDead ) {
        assert( !"this is dead" );
        return;
    }
    if ( m_range > m_maxRange ) {
        setStatus( Status::eDead );
        return;
    }

    switch ( m_type ) {
    case Type::eSlug:
        m_color1[ 3 ] -= 2.0f * updateContext.deltaTime;
        m_range += m_maxRange * 2.0f * updateContext.deltaTime;
        break;

    case Type::eTorpedo:
        if ( m_target && m_target->status() != Status::eDead ) {
            m_direction = interceptTarget( m_direction, m_position, m_target->position(), 160.0_deg * updateContext.deltaTime );
        }
        [[fallthrough]];

    case Type::eBlaster:
        m_prevPosition = m_position;
        m_position += velocity() * updateContext.deltaTime;
        if ( math::length( m_tail[ 1 ] - m_position ) > m_tailChunkLength ) {
            std::rotate( m_tail.begin(), m_tail.end() - 1, m_tail.end() );
        }
        m_tail.front() = position();
        m_range += m_speed * updateContext.deltaTime;
        break;
    }
};

void Bullet::setDirection( const math::vec3& v )
{
    m_direction = math::normalize( v );
    if ( m_type == Type::eSlug ) {
        std::rotate( m_tail.begin(), m_tail.end() - 1, m_tail.end() );
        m_tail.front() = position() + direction() * 1000.0f;
    }
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
