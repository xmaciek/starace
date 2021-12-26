#include "targeting.hpp"

#include "circle.hpp"
#include "colors.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"

#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>

#include <array>

static constexpr std::array target = {
    glm::vec4{ -64.0f, -64.0f, 0.0f, 0.0f },
    glm::vec4{ 64.0f, -64.0f, 0.0f, 0.0f },

    glm::vec4{ -64.0f, 64.0f, 0.0f, 0.0f },
    glm::vec4{ 64.0f, 64.0f, 0.0f, 0.0f },

    glm::vec4{ 64.0f, 64.0f, 0.0f, 0.0f },
    glm::vec4{ 64.0f, -64.0f, 0.0f, 0.0f },

    glm::vec4{ -64.0f, 64.0f, 0.0f, 0.0f },
    glm::vec4{ -64.0f, -64.0f, 0.0f, 0.0f },

};

void Targeting::render( RenderContext rctx ) const
{
    if ( !m_pos ) {
        return;
    }
    const glm::vec3 pos2d = project3dTo2d( rctx.camera3d, *m_pos, rctx.viewport );
    if ( !isOnScreen( pos2d, rctx.viewport ) ) {
        return;
    }
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dColor1 ),
        .m_useLineWidth = true,
        .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dColor1> ),
        .m_verticeCount = target.size(),
        .m_lineWidth = 2.0f,
    };

    PushConstant<Pipeline::eLine3dColor1> pushConstant{};
    pushConstant.m_color = color::winScreen;
    pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ pos2d.x, rctx.viewport.y - pos2d.y, 0.0f } );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    std::copy_n( target.begin(), target.size(), pushConstant.m_vertices.begin() );

    rctx.renderer->push( pushBuffer, &pushConstant );
}

void Targeting::setPos( glm::vec3 v )
{
    m_pos = v;
}

void Targeting::hide()
{
    m_pos.reset();
}
