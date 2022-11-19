#include "skybox.hpp"

#include "game_pipeline.hpp"
#include "map_create_info.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

static constexpr float uvmin = 0.00125f;
static constexpr float uvmax = 0.99875f;
static constexpr float size = 1000.0f;

static constexpr std::array<math::vec4, 4> wall1{
    math::vec4{ -size, -size, size, 0.0f },
    math::vec4{  size, -size, size, 0.0f },
    math::vec4{  size,  size, size, 0.0f },
    math::vec4{ -size,  size, size, 0.0f }
};
static constexpr std::array<math::vec4, 4> wall2{
    math::vec4{ -size,  size, -size, 0.0f },
    math::vec4{  size,  size, -size, 0.0f },
    math::vec4{  size, -size, -size, 0.0f },
    math::vec4{ -size, -size, -size, 0.0f }
};
static constexpr std::array<math::vec4, 4> wall3{
    math::vec4{ -size, -size, -size, 0.0f },
    math::vec4{ -size, -size, size, 0.0f },
    math::vec4{ -size,  size, size, 0.0f },
    math::vec4{ -size,  size, -size, 0.0f }
};
static constexpr std::array<math::vec4, 4> wall4{
    math::vec4{  size,  size, -size, 0.0f },
    math::vec4{  size,  size, size, 0.0f },
    math::vec4{  size, -size, size, 0.0f },
    math::vec4{  size, -size, -size, 0.0f }
};
static constexpr std::array<math::vec4, 4> wall5{
    math::vec4{ -size,  size, -size, 0.0f },
    math::vec4{ -size,  size, size, 0.0f },
    math::vec4{  size,  size, size, 0.0f },
    math::vec4{  size,  size, -size, 0.0f }
};
static constexpr std::array<math::vec4, 4> wall6{
    math::vec4{ -size, -size, -size, 0.0f },
    math::vec4{  size, -size, -size, 0.0f },
    math::vec4{  size, -size, size, 0.0f },
    math::vec4{ -size, -size, size, 0.0f }
};

static constexpr std::array<math::vec4,4> uv1 {
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<math::vec4,4> uv2 {
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f }
};
static constexpr std::array<math::vec4,4> uv3 {
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<math::vec4,4> uv4 {
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f }
};
static constexpr std::array<math::vec4,4> uv5 {
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};
static constexpr std::array<math::vec4,4> uv6 {
    math::vec4{ uvmin, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmin, 0.0f, 0.0f },
    math::vec4{ uvmax, uvmax, 0.0f, 0.0f },
    math::vec4{ uvmin, uvmax, 0.0f, 0.0f }
};

void Skybox::render( const RenderContext& rctx ) const
{
    using Wall = MapCreateInfo::Wall;
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dTexture ),
        .m_verticeCount = 4,
    };
    PushConstant<Pipeline::eTriangleFan3dTexture> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eBack ];
    std::copy_n( wall1.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv1.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eFront ];
    std::copy_n( wall2.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv2.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eLeft ];
    std::copy_n( wall3.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv3.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eRight ];
    std::copy_n( wall4.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv4.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eTop ];
    std::copy_n( wall5.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv5.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );

    pushBuffer.m_resource[ 1 ].texture = m_texture[ Wall::eBottom ];
    std::copy_n( wall6.begin(), 4, pushConstant.m_vertices.begin() );
    std::copy_n( uv6.begin(), 4, pushConstant.m_uv.begin() );
    rctx.renderer->push( pushBuffer, &pushConstant );
}
