#include "progressbar.hpp"

#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

ProgressBar::ProgressBar( glm::vec2 pos, glm::vec2 size, glm::vec2 axis, const glm::vec4 colorA, const glm::vec4 colorB ) noexcept
: Widget{ pos, size }
, m_colorA{ colorA }
, m_colorB{ colorB }
, m_axis{ axis }
{
}

void ProgressBar::render( RenderContext rctx ) const
{
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eProgressBar ),
        .m_pushConstantSize = sizeof( PushConstant<Pipeline::eProgressBar> ),
        .m_verticeCount = 4,
    };
    PushConstant<Pipeline::eProgressBar> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    const glm::vec2 pos = position() + offsetByAnchor();
    const float x = pos.x;
    const float y = pos.y;
    const float xw = x + m_size.x;
    const float yh = y + m_size.y;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ x, yh, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ xw, yh, 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ xw, y, 0.0f, 0.0f };

    pushConstant.m_color[ 0 ] = m_colorA;
    pushConstant.m_color[ 1 ] = m_colorB;
    pushConstant.m_axis = glm::vec4{ m_axis, 0.0f, m_value };
    rctx.renderer->push( pushBuffer, &pushConstant );
}

void ProgressBar::setValue( float f )
{
    m_value = f;
}
