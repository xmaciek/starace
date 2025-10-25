#include "glow.hpp"

#include <ui/property.hpp>
#include <ui/pipeline.hpp>

#include <renderer/renderer.hpp>

namespace ui {

void Glow::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eGlow>;
    RenderInfo ri{
        .m_pipeline = m_pipeline,
        .m_verticeCount = PushConstant::VERTICES,
    };
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = rctx.colorMain,
    };

    const math::vec2 pos = position() + offsetByAnchor();
    const float x = pos.x;
    const float y = pos.y;
    const float xw = x + m_size.x;
    const float yh = y + m_size.y;

    pushConstant.m_xyuv[ 0 ] = math::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_xyuv[ 1 ] = math::vec4{ x, yh, 0.0f, 1.0f };
    pushConstant.m_xyuv[ 2 ] = math::vec4{ xw, yh, 1.0f, 1.0f };
    pushConstant.m_xyuv[ 3 ] = math::vec4{ xw, y, 1.0f, 0.0f };

    ri.m_uniform = pushConstant;
    rctx.renderer->render( ri );
}

}
