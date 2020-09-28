#include "thruster.hpp"

#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

Buffer Thruster::s_innerCone[ 2 ]{};
Buffer Thruster::s_outerCone[ 2 ]{};

Thruster::Thruster( float length, float radius )
: m_inner( 32, radius * 0.6 )
, m_outer( 32, radius )
{
    setLength( length );
}

void Thruster::setLength( float newLength )
{
    m_lengthRange = glm::vec2{ newLength * 0.95f, newLength };
}

void Thruster::setColorScheme( const ColorScheme& col )
{
    m_colorScheme = col;
}

void Thruster::update( const UpdateContext& updateContext )
{
    constexpr static float wiggleDuration = 0.25f;
    m_wiggle = std::fmod( m_wiggle + updateContext.deltaTime, wiggleDuration );
    const float f = std::cos( (float)M_PI * ( m_wiggle / wiggleDuration - 0.5f ) );
    const float diff = m_lengthRange.y - m_lengthRange.x;
    m_length = m_lengthRange.x + f * diff;
}

static Buffer makeBuffer( Renderer* renderer, const Circle& c )
{
    assert( renderer );
    std::pmr::vector<glm::vec3> cone{ renderer->allocator() };
    cone.reserve( c.segments() + 2 );
    cone.emplace_back( 0.0f, 0.0f, 1.0f );
    for ( size_t i = 0; i < c.segments() ; ++i ) {
        cone.emplace_back( c.x( i ), c.y( i ), 0.0f );
    }
    cone.emplace_back( c.x( 0 ), c.y( 0 ), 0.0f );
    std::reverse( cone.begin() + 1, cone.end() );
    return renderer->createBuffer( std::move( cone ), Buffer::Lifetime::ePersistent );
}

static Buffer makeColorBuffer( Renderer* renderer, uint32_t count, const glm::vec4& a, const glm::vec4& b )
{
    assert( renderer );
    std::pmr::vector<glm::vec4> colors{ renderer->allocator() };
    colors.resize( count, a );
    colors.front() = b;
    return renderer->createBuffer( std::move( colors ), Buffer::Lifetime::ePersistent );
}

void Thruster::renderAt( RenderContext rctx, const glm::vec3& pos ) const
{
    if ( !s_innerCone[ 0 ] ) {
        s_innerCone[ 0 ] = makeBuffer( rctx.renderer, m_inner );
        s_innerCone[ 1 ] = makeColorBuffer( rctx.renderer, m_inner.segments() + 2, m_colorScheme[ 0 ], m_colorScheme[ 1 ] );
        s_outerCone[ 0 ] = makeBuffer( rctx.renderer, m_outer );
        s_outerCone[ 1 ] = makeColorBuffer( rctx.renderer, m_outer.segments() + 2, m_colorScheme[ 2 ], m_colorScheme[ 3 ] );
    }

    PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
    pushConstant.m_model = glm::translate( rctx.model, pos );
    pushConstant.m_model = glm::scale( pushConstant.m_model, glm::vec3{ 1.0f, 1.0f, m_length } );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangleFan3dColor> pushInner{};
    PushBuffer<Pipeline::eTriangleFan3dColor> pushOuter{};

    pushInner.m_vertices = s_innerCone[ 0 ];
    pushInner.m_colors = s_innerCone[ 1 ];
    pushOuter.m_vertices = s_outerCone[ 0 ];
    pushOuter.m_colors = s_outerCone[ 1 ];

    rctx.renderer->push( &pushInner, &pushConstant );
    rctx.renderer->push( &pushOuter, &pushConstant );
}
