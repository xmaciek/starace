#include <ui/font.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <span>
#include <memory_resource>
#include <vector>

#define HACK_SIZE * 0.5f

static std::tuple<math::vec4, math::vec4> composeSprite( const ui::Font::Glyph* glyph, math::vec2 extent, math::vec2 surfacePos, math::vec2 originPointHack )
{
    math::vec2 position{ glyph->position[ 0 ], glyph->position[ 1 ] };
    math::vec2 padding = math::vec2{ glyph->padding[ 0 ], glyph->padding[ 1 ] } HACK_SIZE;
    math::vec2 size{ glyph->size[ 0 ], glyph->size[ 1 ] };
    math::vec2 uv1 = position / extent;
    math::vec2 uv2 = size / extent;

    // HACK: also hack for sizing
    size *= 0.5f;
    padding.y *= -1.0f;
    padding += originPointHack; // top vs bottom
    math::vec2 topLeft = surfacePos + padding;

    return std::make_tuple(
        math::vec4{ topLeft.x, topLeft.y, size.x, size.y },
        math::vec4{ uv1.x, uv1.y, uv2.x, uv2.y }
    );
}

struct FontAtlasHeader {
    static constexpr inline uint32_t MAGIC = 'ATNF';
    static constexpr inline uint32_t VERSION = 'ATNF';
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t count = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

namespace ui {

Font::Font( const CreateInfo& ci )
: m_lineHeight{ ci.lineHeight }
, m_texture{ ci.texture }
{
    ZoneScoped;
    assert( !ci.fontAtlas.empty() );
    assert( ci.lineHeight > 0 );
    assert( ci.texture );

    const uint8_t* ptr = ci.fontAtlas.data();
    FontAtlasHeader header{};
    std::memcpy( &header, ptr, sizeof( header ) );
    std::advance( ptr, sizeof( header ) );

    const char32_t* charsBegin = reinterpret_cast<const char32_t*>( ptr );
    const char32_t* charsEnd = charsBegin + header.count;
    const Glyph* glyphsBegin = reinterpret_cast<const Glyph*>( charsEnd );
    const Glyph* glyphsEnd = glyphsBegin + header.count;

    std::span<const char32_t> charSpan{ charsBegin, charsEnd };
    std::span<const Glyph> glyphSpan{ glyphsBegin, glyphsEnd };
    m_width = header.width;
    m_height = header.height;
    m_glyphMap = GlyphMap{ charSpan, glyphSpan };

}

uint32_t Font::height() const
{
    return m_lineHeight;
}

float Font::textLength( std::u32string_view text ) const
{
    float ret = 0.0f;
    const auto sum = [this]( auto ch ) -> float
    {
        const Glyph* glyph = m_glyphMap.find( ch );
        assert( glyph );
        return static_cast<float>( glyph->advance[ 0 ] ) HACK_SIZE;
    };
    for ( auto ch : text ) { ret += sum( ch ); }
    return ret;
}

Font::RenderText Font::composeText( const math::vec4& color, std::u32string_view text ) const
{
    ZoneScoped;
    assert( text.size() < ui::PushConstant<ui::Pipeline::eSpriteSequence>::INSTANCES );
    const uint32_t count = static_cast<uint32_t>( text.size() );

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = 6,
        .m_instanceCount = count,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    ui::PushConstant<ui::Pipeline::eSpriteSequence> pushConstant{};
    pushConstant.m_color = color;
    math::vec2 surfacePos{};
    math::vec2 extent{ m_width, m_height };

    for ( uint32_t i = 0; i < count; ++i ) {
        char32_t chr = text[ i ];
        const Glyph* glyph = m_glyphMap.find( chr );
        assert( glyph );
        auto& sprite = pushConstant.m_sprites[ i ];
        std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph, extent, surfacePos, { 0.0f, m_lineHeight } );
        surfacePos.x += (float)glyph->advance[ 0 ] HACK_SIZE;
    }
    return { pushData, pushConstant };
}

}
