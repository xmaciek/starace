#include "shield.hpp"

#include <renderer/buffer.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

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
    static Buffer vertices{};
    if ( vertices == Buffer::Status::eNone ) {
        std::pmr::vector<glm::vec3> vec{ rctx.renderer->allocator() };
        vec.reserve( m_circle.segments() + 1 );
        for ( uint32_t i = 0; i < m_circle.segments(); ++i ) {
            vec.emplace_back( m_circle.x( i ), m_circle.y( i ), m_radius );
        }
        vec.emplace_back( m_circle.x( 0 ), m_circle.y( 0 ), m_radius );
        vertices = rctx.renderer->createBuffer( std::move( vec ), Buffer::Lifetime::ePersistent );
        assert( vertices != Buffer::Status::eNone );
    }

    PushBuffer<Pipeline::eLine3dStripColor1> pushBuffer{};
    pushBuffer.m_lineWidth = 1.0f;
    pushBuffer.m_vertices = vertices;

    PushConstant<Pipeline::eLine3dStripColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = m_color;

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
