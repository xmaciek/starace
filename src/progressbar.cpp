#include "progressbar.hpp"

#include "colors.hpp"

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

Progressbar::Progressbar( const Progressbar::CreateInfo& ci ) noexcept
: Widget{ ci.position, {} }
, m_dataModel{ ci.model }
{
}

void Progressbar::render( ui::RenderContext rctx ) const
{
    static constexpr uint32_t INSTANCES = 16u;
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = 6u,
        .m_instanceCount = INSTANCES,
    };
    pushData.m_resource[ 1 ].texture = g_uiProperty.atlasTexture();

    using PushConstant = ui::PushConstant<ui::Pipeline::eSpriteSequenceColors>;
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    math::vec2 pos = position() + offsetByAnchor();
    auto Gen = [pos, i = 0u, value = m_value]() mutable
    {
        auto j = i++;
        return PushConstant::Sprite{
            .m_color = (float)j < ( value * (float)INSTANCES ) ? color::winScreen : color::crimson,
            .m_xywh{ pos.x + ( j * 10.0f ), pos.y, 16.0f, 16.0f },
            .m_uvwh = math::makeUVxywh<8, 8>( 2, 3 ),
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