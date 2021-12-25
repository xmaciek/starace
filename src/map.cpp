#include "map.hpp"

#include <algorithm>

#include "utils.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static constexpr float uvmin = 0.00125f;
static constexpr float uvmax = 0.99875f;
static constexpr float size = 1000.0f;

static constexpr std::array<glm::vec4, 4> wall1{
    glm::vec4{ -size, -size, size, 0.0f },
    glm::vec4{  size, -size, size, 0.0f },
    glm::vec4{  size,  size, size, 0.0f },
    glm::vec4{ -size,  size, size, 0.0f }
};
static constexpr std::array<glm::vec4, 4> wall2{
    glm::vec4{ -size,  size, -size, 0.0f },
    glm::vec4{  size,  size, -size, 0.0f },
    glm::vec4{  size, -size, -size, 0.0f },
    glm::vec4{ -size, -size, -size, 0.0f }
};
static constexpr std::array<glm::vec4, 4> wall3{
    glm::vec4{ -size, -size, -size, 0.0f },
    glm::vec4{ -size, -size, size, 0.0f },
    glm::vec4{ -size,  size, size, 0.0f },
    glm::vec4{ -size,  size, -size, 0.0f }
};
static constexpr std::array<glm::vec4, 4> wall4{
    glm::vec4{  size,  size, -size, 0.0f },
    glm::vec4{  size,  size, size, 0.0f },
    glm::vec4{  size, -size, size, 0.0f },
    glm::vec4{  size, -size, -size, 0.0f }
};
static constexpr std::array<glm::vec4, 4> wall5{
    glm::vec4{ -size,  size, -size, 0.0f },
    glm::vec4{ -size,  size, size, 0.0f },
    glm::vec4{  size,  size, size, 0.0f },
    glm::vec4{  size,  size, -size, 0.0f }
};
static constexpr std::array<glm::vec4, 4> wall6{
    glm::vec4{ -size, -size, -size, 0.0f },
    glm::vec4{  size, -size, -size, 0.0f },
    glm::vec4{  size, -size, size, 0.0f },
    glm::vec4{ -size, -size, size, 0.0f }
};

static constexpr std::array<glm::vec4,4> uv1 {
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<glm::vec4,4> uv2 {
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f }
};
static constexpr std::array<glm::vec4,4> uv3 {
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<glm::vec4,4> uv4 {
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f }
};
static constexpr std::array<glm::vec4,4> uv5 {
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<glm::vec4,4> uv6 {
    glm::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    glm::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    glm::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};

void Map::render( RenderContext rctx )
{
    // NOTE: this is so bad, should ba model rendering
    {
        using Wall = MapCreateInfo::Wall;
        PushBuffer pushBuffer{
            .m_pipeline = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dTexture ),
            .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dTexture> ),
            .m_verticeCount = 4,
        };
        PushConstant<Pipeline::eTriangleFan3dTexture> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;

        pushBuffer.m_texture = m_texture[ Wall::eBack ];
        std::copy_n( wall1.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv1.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );

        pushBuffer.m_texture = m_texture[ Wall::eFront ];
        std::copy_n( wall2.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv2.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );

        pushBuffer.m_texture = m_texture[ Wall::eLeft ];
        std::copy_n( wall3.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv3.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );

        pushBuffer.m_texture = m_texture[ Wall::eRight ];
        std::copy_n( wall4.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv4.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );

        pushBuffer.m_texture = m_texture[ Wall::eTop ];
        std::copy_n( wall5.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv5.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );

        pushBuffer.m_texture = m_texture[ Wall::eBottom ];
        std::copy_n( wall6.begin(), 4, pushConstant.m_vertices.begin() );
        std::copy_n( uv6.begin(), 4, pushConstant.m_uv.begin() );
        rctx.renderer->push( pushBuffer, &pushConstant );
    }

    {
        PushConstant<Pipeline::eLine3dColor1> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_color = glm::vec4{ 1.0f, 1.0f, 1.0f, 0.4f };

        PushBuffer pushBuffer{
            .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dColor1 ),
            .m_useLineWidth = true,
            .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dColor1> ),
            .m_verticeCount = static_cast<uint32_t>( m_particleList.size() * 2 ),
            .m_lineWidth = 1.0f,
        };
        assert( pushBuffer.m_verticeCount <= pushConstant.m_vertices.size() );

        size_t i = 0;
        for ( const glm::vec4& it : m_particleList ) {
            pushConstant.m_vertices[ i++ ] = it;
            pushConstant.m_vertices[ i++ ] = it + m_particleLength;
        }
        rctx.renderer->push( pushBuffer, &pushConstant );
    }
}

void Map::update( const UpdateContext& updateContext )
{
    const glm::vec4 tmpVelocity{ m_jetVelocity * -0.1f * updateContext.deltaTime, 0.0f };
    for ( auto& it : m_particleList ) {
        it += tmpVelocity;
        if ( glm::distance( glm::vec3{ it }, m_jetPosition ) >= 1.5 ) {
            it.x = randomRange( m_jetPosition.x - 1, m_jetPosition.x + 1 );
            it.y = randomRange( m_jetPosition.y - 1, m_jetPosition.y + 1 );
            it.z = randomRange( m_jetPosition.z - 1, m_jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapCreateInfo& data )
: m_texture{ data.texture }
{
    m_particleList.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        const float x = randomRange( -1, 1 );
        const float y = randomRange( -1, 1 );
        const float z = randomRange( -1, 1 );
        m_particleList.emplace_back( x, y, z, 0.0f );
    }
}

void Map::setJetData( const glm::vec3& position, const glm::vec3& velocity )
{
    m_jetPosition = position;
    m_jetVelocity = velocity;
    m_particleLength = glm::vec4{ m_jetVelocity * 0.05f, 0.0f };
}
