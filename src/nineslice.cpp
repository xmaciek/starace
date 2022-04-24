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


void NineSlice::render( RenderContext rctx ) const
{
    ZoneScoped;
    using Generator = spritegen::NineSliceComposer;
    assert( m_texture );
    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eSpriteSequence ),
        .m_verticeCount = Generator::count(),
        .m_texture = m_texture,
    };

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    const math::vec2 midWH{ s.x - ( m_left + m_right ), s.y - ( m_top + m_bot ) };

    PushConstant<Pipeline::eSpriteSequence> pushConstant{};
    assert( Generator::count() <= pushConstant.m_xyuv.size() );
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = m_color;

    Generator nineSliceComposer{
        .m_atlas = m_atlas,
        .m_spriteIds = m_spriteIds,
        .m_xyBegin = math::vec4{ pos.x, pos.y, 0, 0 },
        .m_midStretch = midWH,
    };
    std::generate_n( pushConstant.m_xyuv.begin(), Generator::count(), nineSliceComposer );

    rctx.renderer->push( pushBuffer, &pushConstant );
}

void NineSlice::setColor( math::vec4 c )
{
    m_color = c;
}

}
