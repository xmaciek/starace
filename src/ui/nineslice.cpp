#include <ui/nineslice.hpp>

#include <ui/font.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>
#include <ui/spritegen.hpp>

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <cassert>

namespace ui {

NineSlice::NineSlice( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_spriteIds{ ci.spriteArray }
{
}


void NineSlice::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eSpriteSequence>;
    ZoneScoped;

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = 9,
    };
    pushData.m_fragmentTexture[ 1 ] = g_uiProperty.atlasTexture();

    const math::vec2 pos{};
    const math::vec2 s = size();
    const math::vec4 mid{ pos.x, pos.y, s.x, s.y };

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = isFocused() ? rctx.colorFocus : rctx.colorMain,
    };

    const auto& atlasRef = *g_uiProperty.atlas();
    math::vec2 atlasExtent = atlasRef.extent();
    NineSlice2 gen{ mid, g_uiProperty.atlas(), m_spriteIds };

    for ( auto i = 0u; i < 9u; ++i ) {
        auto& sprite = pushConstant.m_sprites[ i ];
        sprite.m_xywh = gen( i );
        sprite.m_uvwh = atlasRef[ m_spriteIds[ i ] ] / atlasExtent;
    }
    rctx.renderer->push( pushData, &pushConstant );
}

}
