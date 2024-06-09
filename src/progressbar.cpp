#include "progressbar.hpp"

#include "colors.hpp"

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

Progressbar::Progressbar( const Progressbar::CreateInfo& ci ) noexcept
: Widget{ ci.position, {} }
, m_spacing{ ci.spriteSpacing }
{
    m_dataModel = g_uiProperty.dataModel( ci.data );
    std::tie( m_uvwh, m_w, m_h, m_texture ) = g_uiProperty.sprite( ci.spriteId );
    m_texture = g_uiProperty.atlasTexture();
}

void Progressbar::render( ui::RenderContext rctx ) const
{
    static constexpr uint32_t INSTANCES = 16u;
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = 6u,
        .m_instanceCount = INSTANCES,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    using PushConstant = ui::PushConstant<ui::Pipeline::eSpriteSequenceColors>;
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    math::vec2 pos = position() + offsetByAnchor();
    const float xOffset = (float)m_w + m_spacing;
    auto Gen = [this, pos, i = 0u, value = m_value, xOffset]() mutable
    {
        auto j = i++;
        return PushConstant::Sprite{
            .m_color = (float)j < ( value * (float)INSTANCES ) ? color::winScreen : color::crimson,
            .m_xywh{ pos.x + ( j * xOffset ), pos.y, m_w, m_h },
            .m_uvwh = m_uvwh,
        };
    };
    std::generate_n( pushConstant.m_sprites.begin(), INSTANCES, Gen );

    rctx.renderer->push( pushData, &pushConstant );
}

void Progressbar::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto idx = m_dataModel->current();
    if ( idx == m_current ) { return; }
    m_current = idx;
    m_value = m_dataModel->atF( idx );
}