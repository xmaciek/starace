#include "hudbar.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "progressbar.hpp"
#include "ui_property.hpp"

#include <renderer/renderer.hpp>
#include <engine/math.hpp>

HudBar::HudBar( std::u32string_view txt ) noexcept
: Widget{ {}, { 64, 112 }, Anchor::fBottom | Anchor::fLeft }
, m_label{ txt, g_uiProperty.fontSmall(), Anchor::fCenter | Anchor::fTop, { 32, 0 }, color::winScreen }
{}

void HudBar::render( RenderContext rctx ) const
{
    const math::vec4 xywh = math::vec4{ position() + offsetByAnchor(), size() };
    const float topoff = m_label.size().y + 4.0f;
    const float sideoff = 8.0f;

    rctx.model = math::translate( rctx.model, math::vec3{ xywh.x, xywh.y, 0.0f } );
    m_label.render( rctx );
    // outline
    {
        PushBuffer pushBuffer{
            .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dStripColor ),
            .m_verticeCount = 4,
            .m_lineWidth = 2.0f,
        };

        const math::vec2 xy{ sideoff, topoff };
        const math::vec2 wh{ xywh.z - sideoff * 2.0f, xywh.w - topoff - 2.0f };
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_vertices[ 0 ] = math::vec4{ xy.x, xy.y + wh.y, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = math::vec4{ xy.x + wh.x, xy.y + wh.y, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = math::vec4{ xy.x + wh.x, xy.y, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = math::vec4{ xy.x, xy.y, 0.0f, 0.0f };
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        std::fill_n( pushConstant.m_colors.begin(), 4, color::winScreen );
        rctx.renderer->push( pushBuffer, &pushConstant );
    }

    // colorbar
    {
        const math::vec2 xy{ sideoff + 2.0f, topoff + 4.0f };
        const math::vec2 wh{ xywh.z - sideoff * 2.0f - 6.0f, ( xywh.w - topoff - 10.0f ) };
        const float red =   m_value < 0.5f ? 1.0f : 1.0f - ( m_value - 0.5f ) * 2.0f;
        const float green = m_value > 0.5f ? 1.0f : m_value * 2.0f;
        const math::vec4 color{ red, green, 0.0f, 1.0f };
        ProgressBar progress{ xy, wh, axis::y, {}, color };
        progress.setValue( 1.0f - m_value );
        progress.render( rctx );
    }
}

void HudBar::setValue( float f )
{
    m_value = f;
}
