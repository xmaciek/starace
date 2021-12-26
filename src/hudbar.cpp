#include "hudbar.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"

#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

HudBar::HudBar( std::u32string_view txt, Font* font ) noexcept
: Widget{ {}, { 64, 112 }, Anchor::fBottom | Anchor::fLeft }
, m_label{ txt, font, Anchor::fCenter | Anchor::fTop, { 32, 0 }, color::winScreen }
{}

void HudBar::render( RenderContext rctx ) const
{
    const glm::vec4 xywh = glm::vec4{ position() + offsetByAnchor(), size() };
    const float topoff = m_label.size().y + 4.0f;
    const float sideoff = 8.0f;

    constexpr auto composeRect = []( glm::vec2 xy, glm::vec2 wh )
    {
        return std::array<glm::vec4, 4>{
            glm::vec4{ xy.x, xy.y + wh.y, 0.0f, 0.0f },
            glm::vec4{ xy.x + wh.x, xy.y + wh.y, 0.0f, 0.0f },
            glm::vec4{ xy.x + wh.x, xy.y, 0.0f, 0.0f },
            glm::vec4{ xy.x, xy.y, 0.0f, 0.0f }
        };
    };
    rctx.model = glm::translate( rctx.model, glm::vec3{ xywh.x, xywh.y, 0.0f } );
    m_label.render( rctx );
    // outline
    {
        PushBuffer pushBuffer{
            .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dStripColor ),
            .m_useLineWidth = true,
            .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dStripColor> ),
            .m_verticeCount = 4,
            .m_lineWidth = 2.0f,
        };

        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        const auto outline = composeRect( { sideoff, topoff }, { xywh.z - sideoff * 2.0f, xywh.w - topoff - 2.0f } );
        std::copy( outline.begin(), outline.end(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), 4, color::winScreen );
        rctx.renderer->push( pushBuffer, &pushConstant );
    }

    // colorbar
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
        const glm::vec2 wh{ xywh.z - sideoff * 2.0f - 6.0f, ( xywh.w - topoff - 10.0f ) };
        const glm::vec2 xy{ sideoff + 2.0f, topoff + 4.0f };
        const auto bar = composeRect( xy, wh );
        std::copy( bar.begin(), bar.end(), std::begin( pushConstant.m_vertices ) );
        pushConstant.m_color[ 1 ] = glm::vec4{
            1.0f - m_value + colorHalf( m_value )
            , m_value + colorHalf( m_value )
            , 0.0f
            , 1.0f
        };
        pushConstant.m_axis = glm::vec4{ axis::y, m_value };
        rctx.renderer->push( pushBuffer, &pushConstant );
    }
}

void HudBar::setValue( float f )
{
    m_value = f;
}
