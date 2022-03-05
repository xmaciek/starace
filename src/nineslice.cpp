#include "nineslice.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <renderer/renderer.hpp>

#include <cassert>
#include <optional>

NineSlice::NineSlice(
    math::vec2 position
    , math::vec2 extent
    , math::vec4 color
    , Anchor a
    , const LinearAtlas* atlas
    , std::array<uint32_t, 9> ns
    , Texture t
) noexcept
: Widget{ position, extent, a }
, m_color{ color }
, m_atlas{ atlas }
, m_texture{ t }
, m_spriteIds{ ns }
{
    m_top = static_cast<float>( m_atlas->sprite( m_spriteIds[ 0u ] )[ 3 ] );
    m_left = static_cast<float>( m_atlas->sprite( m_spriteIds[ 0u ] )[ 2 ] );
    m_right = static_cast<float>( m_atlas->sprite( m_spriteIds[ 2u ] )[ 2 ] );
    m_bot = static_cast<float>( m_atlas->sprite( m_spriteIds[ 6u ] )[ 3 ] );
}


template <uint32_t args>
float copyIn( auto& it, Sprite sprite, math::vec4 uv, math::vec2 pos, math::vec2 overrideWH )
{
    const float ww = ( args & 0b10 ) ? overrideWH.x : static_cast<float>( sprite[ 2 ] );
    const float hh = ( args & 0b01 ) ? overrideWH.y : static_cast<float>( sprite[ 3 ] );
    const math::vec4 xywh{ pos.x, pos.y, ww, hh };
    auto verts = compose6( xywh, uv );
    std::copy( verts.begin(), verts.end(), it );
    std::advance( it, 6 );
    return ww;
};

void NineSlice::render( RenderContext rctx ) const
{
    assert( m_texture );
    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eSpriteSequence ),
        .m_verticeCount = 54,
        .m_texture = m_texture,
    };

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    const math::vec2 midWH{ s.x - ( m_left + m_right ), s.y - ( m_top + m_bot ) };
    math::vec2 inPos = pos;

    PushConstant<Pipeline::eSpriteSequence> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = m_color;
    auto it = pushConstant.m_xyuv.begin();


    inPos.x += copyIn<0b00>( it, m_atlas->sprite( m_spriteIds[ 0u ] ), m_atlas->sliceUV( m_spriteIds[ 0u ] ), inPos, midWH );
    inPos.x += copyIn<0b10>( it, m_atlas->sprite( m_spriteIds[ 1u ] ), m_atlas->sliceUV( m_spriteIds[ 1u ] ), inPos, midWH );
               copyIn<0b00>( it, m_atlas->sprite( m_spriteIds[ 2u ] ), m_atlas->sliceUV( m_spriteIds[ 2u ] ), inPos, midWH );

    inPos.x = pos.x;
    inPos.y += m_top;

    inPos.x += copyIn<0b01>( it, m_atlas->sprite( m_spriteIds[ 3u ] ), m_atlas->sliceUV( m_spriteIds[ 3u ] ), inPos, midWH );
    inPos.x += copyIn<0b11>( it, m_atlas->sprite( m_spriteIds[ 4u ] ), m_atlas->sliceUV( m_spriteIds[ 4u ] ), inPos, midWH );
               copyIn<0b01>( it, m_atlas->sprite( m_spriteIds[ 5u ] ), m_atlas->sliceUV( m_spriteIds[ 5u ] ), inPos, midWH );

    inPos.x = pos.x;
    inPos.y += midWH.y;

    inPos.x += copyIn<0b00>( it, m_atlas->sprite( m_spriteIds[ 6u ] ), m_atlas->sliceUV( m_spriteIds[ 6u ] ), inPos, midWH );
    inPos.x += copyIn<0b10>( it, m_atlas->sprite( m_spriteIds[ 7u ] ), m_atlas->sliceUV( m_spriteIds[ 7u ] ), inPos, midWH );
               copyIn<0b00>( it, m_atlas->sprite( m_spriteIds[ 8u ] ), m_atlas->sliceUV( m_spriteIds[ 8u ] ), inPos, midWH );


    rctx.renderer->push( pushBuffer, &pushConstant );
}

void NineSlice::setColor( math::vec4 c )
{
    m_color = c;
}

