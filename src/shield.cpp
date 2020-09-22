#include "shield.hpp"

#include "render_pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>

Shield::Shield( double radiusA, double radiusB )
: m_circle( 6, radiusB )
, m_radius( static_cast<float>( radiusA ) )
{
}

void Shield::update( const UpdateContext& updateContext )
{
    m_rotAngle += 60.0f * updateContext.deltaTime;
    if ( m_rotAngle >= 360 ) {
        m_rotAngle -= 360;
    }
}

void Shield::render( RenderContext rctx ) const
{
    PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
    pushConstant.m_model = glm::rotate( rctx.model, glm::radians( m_rotAngle ), glm::vec3{ 0.0f, 1.0f, 0.0f } );
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;

    PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{ rctx.renderer->allocator() };
    pushBuffer.m_colors.resize( m_circle.segments() + 1, m_color );
    pushBuffer.m_vertices.reserve( m_circle.segments() + 1 );
    for ( size_t i = 0; i < m_circle.segments(); ++i ) {
        pushBuffer.m_vertices.emplace_back( m_circle.x( i ), m_circle.y( i ), m_radius );
    }
    pushBuffer.m_vertices.emplace_back( m_circle.x( 0 ), m_circle.y( 0 ), m_radius );

    for ( size_t i = 0; i < 8; ++i ) {
        rctx.renderer->push( &pushBuffer, &pushConstant );
        pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 45.0f ), glm::vec3{ 0, 1, 0 } );
    }
    pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( m_rotAngle ), glm::vec3{ 0, 0, 1 } );
    for ( size_t i = 0; i < 8; ++i ) {
        rctx.renderer->push( &pushBuffer, &pushConstant );
        pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 45.0f ), glm::vec3{ 0.0f, 1.0f, 0.0f } );
    }
    pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( m_rotAngle ), glm::vec3{ 1, 0, 0 } );
    for ( size_t i = 0; i < 8; ++i ) {
        rctx.renderer->push( &pushBuffer, &pushConstant );
        pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 45.0f ), glm::vec3{ 0.0f, 1.0f, 0.0f } );
    }


}

void Shield::setColor( const glm::vec4& c )
{
    m_color = c;
}
