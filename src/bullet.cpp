#include "bullet.hpp"

#include "render_pipeline.hpp"

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cassert>
#include <random>

static uint16_t typeToSegments( Bullet::Type e )
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

Bullet::Bullet( const BulletProto& bp )
: m_tail( typeToSegments( bp.type ), bp.position )
{
    m_collisionFlag = true;
    m_type = bp.type;
    m_speed = bp.speed;
    m_damage = bp.damage;
    std::copy( std::begin( bp.color1 ), std::end( bp.color1 ), std::begin( m_color1 ) );
    std::copy( std::begin( bp.color2 ), std::end( bp.color2 ), std::begin( m_color2 ) );
    m_score = bp.score_per_hit;

    if ( m_type == Type::eSlug ) {
        std::copy( std::begin( m_color1 ), std::end( m_color1 ), std::begin( m_color2 ) );
        m_color1[ 3 ] = 1;
        m_color2[ 3 ] = 0;
    }
    m_position = bp.position;
    m_rotX = 0;
    m_rotY = 0;
    m_rotZ = 0;
    static std::mt19937_64 rng{ std::random_device()() };
    m_rotation = rng() % 360;

    m_range = 0;
    m_maxRange = 150;

    setStatus( Status::eAlive );
    m_ttl = 20;
};

void Bullet::render( RenderContext rctx ) const
{
    if ( status() != Status::eAlive ) {
        return;
    }

    assert( m_tail.size() == typeToSegments( m_type ) );
    switch ( m_type ) {
    case Type::eSlug: {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_vertices.emplace_back( *m_tail.begin() );
        pushBuffer.m_vertices.emplace_back( *( m_tail.begin() + 1 ) );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], m_color1[ 3 ] );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], m_color1[ 3 ] );
        pushBuffer.m_lineWidth = 2.0f;
        rctx.renderer->push( &pushBuffer, &pushConstant );
    } break;

    case Type::eBlaster: {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_vertices.emplace_back( *m_tail.begin() );
        pushBuffer.m_vertices.emplace_back( *( m_tail.begin() + 3 ) );
        pushBuffer.m_vertices.emplace_back( *( m_tail.begin() + 8 ) );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], m_color1[ 3 ] );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], m_color1[ 3 ] );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], 0.0f );
        pushBuffer.m_lineWidth = 2.0f;
        rctx.renderer->push( &pushBuffer, &pushConstant );
    } break;

    case Type::eTorpedo: {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_vertices.resize( m_tail.size() );
        std::copy( m_tail.cbegin(), m_tail.cend(), pushBuffer.m_vertices.begin() );
        pushBuffer.m_colors.reserve( m_tail.size() );
        pushBuffer.m_colors.emplace_back( m_color1[ 0 ], m_color1[ 1 ], m_color1[ 2 ], m_color1[ 3 ] );
        pushBuffer.m_colors.emplace_back( 1, 1, 1, 1 );
        for ( size_t i = 2; i < m_tail.size(); ++i ) {
            pushBuffer.m_colors.emplace_back( 1.0f, 1.0f, 1.0f, 1.0f / i );
        }
        pushBuffer.m_lineWidth = 2.0f;
        rctx.renderer->push( &pushBuffer, &pushConstant );
    } break;
    }
}

void Bullet::draw() const
{
}

void Bullet::update( const UpdateContext& updateContext )
{
    if ( status() == Status::eDead ) {
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
        m_range += m_maxRange * 2.0 * updateContext.deltaTime;
        break;

    case Type::eTorpedo:
        m_turnrate = glm::radians( speed() * 10.0f * updateContext.deltaTime );
        interceptTarget();
        /*no break*/

    case Type::eBlaster:
        m_position += m_velocity * updateContext.deltaTime;
        std::rotate( m_tail.begin(), m_tail.end() - 1, m_tail.end() );
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
    if ( collisionTest( object ) ) {
        object->addScore( score(), false );
        switch ( m_type ) {
        case Type::eSlug:
            object->setDamage( m_damage * m_color1[ 3 ] );
            break;
        default:
            object->setDamage( m_damage );
            setStatus( Status::eDead );
            break;
        }
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

uint32_t Bullet::damage() const
{
    return m_damage;
}

Bullet::Type Bullet::type() const
{
    return m_type;
}
