#include "thruster.hpp"

#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Thruster::Thruster( double length, double radius )
: m_inner( 32, radius * 0.6 )
, m_outer( 32, radius )
{
    setLength( length );
}

void Thruster::setLength( double newLength )
{
    m_length = newLength;
    m_lengthShorter = newLength * 0.95f;
}

void Thruster::setColor( uint32_t num, float* colorData )
{
    if ( num > 3 ) {
        return;
    }
    m_color[ num ] = glm::vec4{ colorData[ 0 ], colorData[ 1 ], colorData[ 2 ], colorData[ 3 ] };
}

void Thruster::update( const UpdateContext& updateContext )
{
    if ( m_wiggle < 0.066f ) {
        m_len = m_length;
    }
    else {
        m_len = m_lengthShorter;
    }
    m_wiggle += updateContext.deltaTime;
    if ( m_wiggle > 0.08f ) {
        m_wiggle = 0;
    }
}

void Thruster::renderAt( RenderContext rctx, const glm::vec3& pos ) const
{
    PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
    pushConstant.m_model = glm::translate( rctx.model, pos );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangleFan3dColor> pushInner{ rctx.renderer->allocator() };
    PushBuffer<Pipeline::eTriangleFan3dColor> pushOuter{ rctx.renderer->allocator() };

    pushInner.m_vertices.reserve( m_inner.segments() + 2 );
    pushInner.m_colors.resize( m_inner.segments() + 2, m_color[ 1 ] );
    pushInner.m_colors.front() = m_color[ 0 ];
    pushOuter.m_vertices.reserve( m_outer.segments() + 2 );
    pushOuter.m_colors.resize( m_outer.segments() + 2, m_color[ 3 ] );
    pushOuter.m_colors.front() = m_color[ 2 ];

    pushInner.m_vertices.emplace_back( 0.0f, 0.0f, m_len );
    for ( size_t i = m_inner.segments() - 1; i > 0; --i ) {
        pushInner.m_vertices.emplace_back( m_inner.x( i ), m_inner.y( i ), 0.0f );
    }
    pushInner.m_vertices.emplace_back( m_inner.x( m_inner.segments() - 1 ), m_inner.y( m_inner.segments() - 1 ), 0.0f );

    pushOuter.m_vertices.emplace_back( 0.0f, 0.0f, m_len );
    for ( size_t i = m_outer.segments() - 1; i > 0; --i ) {
        pushOuter.m_vertices.emplace_back( m_outer.x( i ), m_outer.y( i ), 0.0f );
    }
    pushOuter.m_vertices.emplace_back( m_outer.x( m_outer.segments() - 1 ), m_outer.y( m_outer.segments() - 1 ), 0.0f );

    rctx.renderer->push( &pushInner, &pushConstant );
    rctx.renderer->push( &pushOuter, &pushConstant );
}
