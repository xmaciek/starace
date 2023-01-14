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
    m_top = static_cast<float>( m_atlas->sprite( m_spriteIds[ 0u ] )[ 3 ] );
    m_left = static_cast<float>( m_atlas->sprite( m_spriteIds[ 0u ] )[ 2 ] );
    m_right = static_cast<float>( m_atlas->sprite( m_spriteIds[ 2u ] )[ 2 ] );
    m_bot = static_cast<float>( m_atlas->sprite( m_spriteIds[ 6u ] )[ 3 ] );
}


void NineSlice::render( RenderContext rctx ) const
{
    ZoneScoped;
    using Generator = spritegen::NineSliceComposer;
    assert( m_texture );
    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eSpriteSequence ],
        .m_verticeCount = 6,
        .m_instanceCount = 9,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    const math::vec2 midWH{ s.x - ( m_left + m_right ), s.y - ( m_top + m_bot ) };

    PushConstant<Pipeline::eSpriteSequence> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = rctx.colorMain,
    };

    Generator nineSliceComposer{
        .m_atlas = m_atlas,
        .m_spriteIds = m_spriteIds,
        .m_xyBegin = math::vec4{ pos.x, pos.y, 0, 0 },
        .m_midStretch = midWH,
    };
    auto gen = [&nineSliceComposer]() { return nineSliceComposer(); };
    for ( uint32_t i = 0; i < 9; ++i ) {
        std::generate_n( pushConstant.m_sprites[ i ].m_xyuv.begin(), 6, gen );
    }

    rctx.renderer->push( pushData, &pushConstant );
}

}
