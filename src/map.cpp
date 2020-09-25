#include "map.hpp"

#include "utils.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Map::render( RenderContext rctx )
{
    PushConstant<Pipeline::eTriangleFan3dTexture> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangleFan3dTexture> pushBuffer{ rctx.renderer->allocator() };
    pushBuffer.m_uv.resize( 4 );
    pushBuffer.m_vertices.resize( 4 );

    pushBuffer.m_texture = m_back;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ m_v1, m_v1, -m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_min };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_max };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_front;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ -m_v1, m_v1, m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ m_v1, m_v1, m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ m_v1, -m_v1, m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_max };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_min };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_left;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ -m_v1, m_v1, m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_min };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_max };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_right;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ m_v1, m_v1, m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ m_v1, m_v1, -m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ m_v1, -m_v1, m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_max };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_min };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_top;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ -m_v1, m_v1, m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ m_v1, m_v1, -m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ m_v1, m_v1, m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_min };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_max };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_bottom;
    pushBuffer.m_vertices[ 0 ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
    pushBuffer.m_vertices[ 1 ] = glm::vec3{ m_v1, -m_v1, m_v1 };
    pushBuffer.m_vertices[ 2 ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
    pushBuffer.m_vertices[ 3 ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
    pushBuffer.m_uv[ 0 ] = glm::vec2{ m_min, m_min };
    pushBuffer.m_uv[ 1 ] = glm::vec2{ m_max, m_min };
    pushBuffer.m_uv[ 2 ] = glm::vec2{ m_max, m_max };
    pushBuffer.m_uv[ 3 ] = glm::vec2{ m_min, m_max };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    PushConstant<Pipeline::eLine3dColor1> linePushConstant{};
    linePushConstant.m_model = rctx.model;
    linePushConstant.m_view = rctx.view;
    linePushConstant.m_projection = rctx.projection;
    linePushConstant.m_color = glm::vec4{ 1.0f, 1.0f, 1.0f, 0.4f };
    PushBuffer<Pipeline::eLine3dColor1> linePushBuffer{};

    std::pmr::vector<glm::vec3> particles{ rctx.renderer->allocator() };
    particles.reserve( m_particleList.size() * 2 );
    for ( const glm::vec3& it : m_particleList ) {
        particles.emplace_back( it );
        particles.emplace_back( it + m_particleLength );
    }
    linePushBuffer.m_vertices = rctx.renderer->createBuffer( std::move( particles ) );
    rctx.renderer->push( &linePushBuffer, &linePushConstant );
    rctx.renderer->deleteBuffer( linePushBuffer.m_vertices );
}

void Map::update( const UpdateContext& updateContext )
{
    const glm::vec3 tmpVelocity = m_jetVelocity * -0.1f * updateContext.deltaTime;
    for ( auto& it : m_particleList ) {
        it += tmpVelocity;
        if ( glm::distance( it, m_jetPosition ) >= 1.5 ) {
            it.x = randomRange( m_jetPosition.x - 1, m_jetPosition.x + 1 );
            it.y = randomRange( m_jetPosition.y - 1, m_jetPosition.y + 1 );
            it.z = randomRange( m_jetPosition.z - 1, m_jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapProto& data )
{
    m_top = loadTexture( data.TOP.c_str() );
    m_bottom = loadTexture( data.BOTTOM.c_str() );
    m_left = loadTexture( data.LEFT.c_str() );
    m_right = loadTexture( data.RIGHT.c_str() );
    m_front = loadTexture( data.FRONT.c_str() );
    m_back = loadTexture( data.BACK.c_str() );

    m_min = 0.00125;
    m_max = 0.99875;

    m_particleList.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        m_particleList.emplace_back( randomRange( -1, 1 ), randomRange( -1, 1 ), randomRange( -1, 1 ) );
    }

    m_v1 = 1000;
    m_v2 = 100;
}

void Map::setJetData( const glm::vec3& position, const glm::vec3& velocity )
{
    m_jetPosition = position;
    m_jetVelocity = velocity;
    m_particleLength = m_jetVelocity * 0.05f;
}

Map::~Map()
{
    destroyTexture( m_top );
    destroyTexture( m_bottom );
    destroyTexture( m_left );
    destroyTexture( m_right );
    destroyTexture( m_front );
    destroyTexture( m_back );
}
