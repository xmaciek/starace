#include "bullet.hpp"

#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include "utils.hpp"

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

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
static std::array<glm::vec4, 23> getTailColors() noexcept
{
    std::array<glm::vec4, 23> ret{};
    const glm::vec4 a{ 1.0f, 1.0f, 1.0f, 1.0f };
    const glm::vec4 diff{ 0.0f, 0.0f, 0.0f, -1.0f };
    size_t i = 0;
    const float f = 1.0f / ret.size();
    for ( auto& it : ret ) {
        const float n = f * (float)i++;
        it = a + diff * ( 0.5f - std::cos( n * (float)M_PI ) * 0.5f );
    }
    return ret;
}

Bullet::Bullet( const BulletProto& bp )
: m_tail( typeToSegments( bp.type ), bp.position )
, m_color1{ bp.color1 }
, m_color2{ bp.color2 }
{
    m_collisionFlag = true;
    m_type = bp.type;
    m_speed = bp.speed;
    m_damage = bp.damage;
    m_score = bp.score_per_hit;
    m_tailChunkLength = m_speed / 30.0f;

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

    PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
    pushBuffer.m_lineWidth = 2.0f;
    PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    // culling
    {
        const auto begin = m_tail.begin();
        const auto end = begin + typeToSegments( m_type );
        const glm::mat4 mvp = rctx.projection * rctx.view * rctx.model;
        const glm::vec2 viewport = rctx.viewport;
        const auto onScreen = [ mvp, viewport ] ( const::glm::vec3& v ) {
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
            []( glm::vec3 v ) { return glm::vec4{ v, 0.0f }; }
        );
        pushConstant.m_colors[ 0 ] = m_color1;
        static const std::array<glm::vec4,23> tail = getTailColors();
        std::copy( tail.begin(), tail.end(), pushConstant.m_colors.begin() + 1 );
    } break;
    }

    rctx.renderer->push( &pushBuffer, &pushConstant );
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

    m_seankyDeltaTime = updateContext.deltaTime;
    switch ( m_type ) {
    case Type::eSlug:
        m_color1[ 3 ] -= 2.0f * updateContext.deltaTime;
        m_range += m_maxRange * 2.0f * updateContext.deltaTime;
        break;

    case Type::eTorpedo:
        m_turnrate = glm::radians( speed() * 10.0f * updateContext.deltaTime );
        interceptTarget();
        [[fallthrough]];

    case Type::eBlaster:
        m_position += m_velocity * updateContext.deltaTime;
        if ( glm::length( m_tail[ 1 ] - m_position ) > m_tailChunkLength ) {
            std::rotate( m_tail.begin(), m_tail.end() - 1, m_tail.end() );
        }
        m_tail.front() = position();
        m_range += m_speed * updateContext.deltaTime;
        break;
    }
};

bool Bullet::collisionTest( const SAObject* object ) const
{
    assert( object );
    if ( !object->canCollide() || status() == Status::eDead || object->status() != Status::eAlive ) {
        return false;
    }

    const glm::vec3 collRay = collisionRay();
    const glm::vec3 dir = object->position() - position();
    const float tmp = glm::dot( dir, collRay );
    float dist = 0.0;

    if ( tmp <= 0 ) {
        dist = glm::length( dir );
    }
    else {
        const float tmp2 = glm::dot( collRay, collRay );
        if ( tmp2 <= tmp ) {
            dist = glm::length( dir );
        }
        else {
            const glm::vec3 Pb = position() + ( collRay * ( tmp / tmp2 ) );
            dist = glm::length( object->position() - Pb );
        }
    }

    return dist < ( m_collisionDistance + object->collisionDistance() );
}

void Bullet::processCollision( SAObject* object )
{
    assert( object );
    if ( !collisionTest( object ) ) {
        return;
    }
    object->addScore( score(), false );
    switch ( m_type ) {
    case Type::eSlug:
        break;
    default:
        object->setDamage( m_damage );
        setStatus( Status::eDead );
        break;
    }
}

glm::vec3 Bullet::collisionRay() const
{
    switch ( m_type ) {
    case Type::eBlaster:
        return direction() * speed() * m_seankyDeltaTime * 3.0f;

    case Type::eTorpedo:
        return direction() * speed() * m_seankyDeltaTime;

    case Type::eSlug:
        return direction() * 1000.0f;

    default:
        assert( !"unreachable" );
        return glm::vec3{};
    }
}

void Bullet::setDirection( const glm::vec3& v )
{
    m_direction = glm::normalize( v );
    m_velocity = direction() * speed();
    if ( m_type == Type::eSlug ) {
        std::rotate( m_tail.begin(), m_tail.end() - 1, m_tail.end() );
        m_tail.front() = position() + direction() * 1000.0f;
    }
}

uint8_t Bullet::damage() const
{
    return m_damage;
}

Bullet::Type Bullet::type() const
{
    return m_type;
}
