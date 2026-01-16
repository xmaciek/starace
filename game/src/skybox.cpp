#include "skybox.hpp"

#include "game_pipeline.hpp"
#include "map_create_info.hpp"

#include <renderer/renderer.hpp>

void Skybox::render( const RenderContext& rctx ) const
{
    const PushConstant<Pipeline::eSkybox> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    RenderInfo ri{
        .m_pipeline = g_pipelines[ Pipeline::eSkybox ],
        .m_verticeCount = pushConstant.VERTICES,
        .m_instanceCount = pushConstant.INSTANCES,
        .m_uniform = pushConstant,
    };

    using Wall = MapCreateInfo::Wall;
    ri.m_fragmentTexture[ 0 ] = m_texture[ Wall::eBack ];
    ri.m_fragmentTexture[ 1 ] = m_texture[ Wall::eFront ];
    ri.m_fragmentTexture[ 2 ] = m_texture[ Wall::eLeft ];
    ri.m_fragmentTexture[ 3 ] = m_texture[ Wall::eRight ];
    ri.m_fragmentTexture[ 4 ] = m_texture[ Wall::eTop ];
    ri.m_fragmentTexture[ 5 ] = m_texture[ Wall::eBottom ];
    rctx.renderer->render( ri );
}
