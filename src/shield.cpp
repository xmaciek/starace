#include "shield.hpp"

#include <array>
#include <algorithm>

#include <renderer/buffer.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

static std::array<glm::vec4, 7> getCircle( float radius ) noexcept
{
    std::array<glm::vec4, 7> ret{};
    const float angle = 2 * M_PI / 6;
    for ( int i = 0; i < 7; ++i ) {
        ret[ i ] = {
            std::sin( angle * (float)( i % 6 ) ) * radius,
            std::cos( angle * (float)( i % 6 ) ) * radius,
            0.1f,
            0.0f
        };
    }
    return ret;
}

static const std::array<glm::vec4, 7> circle = getCircle( 0.02f );

Shield::Shield( double radiusA, double )
: m_radius( static_cast<float>( radiusA ) )
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

    PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
    pushBuffer.m_lineWidth = 1.0f;
    pushBuffer.m_verticeCount = circle.size();

    PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    const glm::vec4 color = glm::vec4{ m_color.r, m_color.g, m_color.b, 1.0f };
    std::fill_n( pushConstant.m_colors.begin(), circle.size(), color );
    std::copy_n( circle.begin(), circle.size(), pushConstant.m_vertices.begin() );

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
