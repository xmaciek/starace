#include "nineslice.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"
#include "spritegen.hpp"

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <cassert>

namespace ui {

NineSlice::NineSlice(
    math::vec2 position
    , math::vec2 extent
    , Anchor a
    , const LinearAtlas* atlas
    , std::array<uint32_t, 9> ns
    , Texture t
) noexcept
: Widget{ position, extent, a }
, m_atlas{ atlas }
, m_texture{ t }
, m_spriteIds{ ns }
{
}


void NineSlice::render( RenderContext rctx ) const
{
    ZoneScoped;

    assert( m_texture );
    assert( m_atlas );

    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eSpriteSequence ],
        .m_verticeCount = 6,
        .m_instanceCount = 9,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    const math::vec4 mid{ pos.x, pos.y, s.x, s.y };

    PushConstant<Pipeline::eSpriteSequence> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = rctx.colorMain,
    };

    spritegen::NineSlice2 gen{ mid, m_atlas, m_spriteIds };
    for ( auto i = 0u; i < 9u; ++i ) {
        auto& sprite = pushConstant.m_sprites[ i ];
        sprite.m_xywh = gen( i );
        sprite.m_uvwh = m_atlas->sliceUV( m_spriteIds[ i ] );
    }
    rctx.renderer->push( pushData, &pushConstant );
}

}
