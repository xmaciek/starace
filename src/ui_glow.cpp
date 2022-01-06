#include "ui_glow.hpp"

#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

void Glow::render( RenderContext rctx ) const
{
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eGlow ),
        .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
        .m_verticeCount = 4,
    };
    PushConstant<Pipeline::eGlow> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = m_color;

    const glm::vec2 pos = position() + offsetByAnchor();
    const float x = pos.x;
    const float y = pos.y;
    const float xw = x + m_size.x;
    const float yh = y + m_size.y;

    pushConstant.m_xyuv[ 0 ] = glm::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_xyuv[ 1 ] = glm::vec4{ x, yh, 0.0f, 1.0f };
    pushConstant.m_xyuv[ 2 ] = glm::vec4{ xw, yh, 1.0f, 1.0f };
    pushConstant.m_xyuv[ 3 ] = glm::vec4{ xw, y, 1.0f, 0.0f };

    rctx.renderer->push( pushBuffer, &pushConstant );
}
