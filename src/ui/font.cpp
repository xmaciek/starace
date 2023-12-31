#include <ui/font.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <span>
#include <memory_resource>
#include <vector>

static std::tuple<math::vec4, math::vec4> composeSprite( const fnta::Glyph* glyph, math::vec2 extent, math::vec2 surfacePos, float scale )
{
    math::vec2 position{ glyph->position[ 0 ], glyph->position[ 1 ] };
    math::vec2 padding = math::vec2{ glyph->padding[ 0 ], glyph->padding[ 1 ] } * scale;
    math::vec2 size{ glyph->size[ 0 ], glyph->size[ 1 ] };
    math::vec2 uv1 = position / extent;
    math::vec2 uv2 = size / extent;

    size *= scale;
    math::vec2 topLeft = surfacePos + padding;

    return std::make_tuple(
        math::vec4{ topLeft.x, topLeft.y, size.x, size.y },
        math::vec4{ uv1.x, uv1.y, uv2.x, uv2.y }
    );
}

namespace ui {

Font::Font( const CreateInfo& ci )
: m_scale{ ci.scale }
, m_texture{ ci.texture }
{
    ZoneScoped;
    assert( !ci.fontAtlas.empty() );
    assert( ci.scale > 0.0f );
    assert( ci.texture );

    using Header = fnta::Header;
    const uint8_t* ptr = ci.fontAtlas.data();
    if ( ci.fontAtlas.size() < sizeof( Header ) ) {
        assert( !"buffer size too small for font atlas" );
        return;
    }

    Header header{};
    std::memcpy( &header, ptr, sizeof( header ) );
    std::advance( ptr, sizeof( header ) );

    if ( header.magic != header.MAGIC ) {
        assert( !"invalid font atlas .magic field" );
        return;
    }

    if ( header.version != header.VERSION ) {
        assert( !"font atlas has incorrect version" );
        return;
    }
    size_t esitmatedBufferSize = sizeof( header );
    esitmatedBufferSize += sizeof( char32_t ) * header.count;
    esitmatedBufferSize += sizeof( fnta::Glyph ) * header.count;
    if ( ci.fontAtlas.size() < esitmatedBufferSize ) {
        assert( !"not enough data in font atlas resource to load" );
        return;
    }

    const char32_t* charsBegin = reinterpret_cast<const char32_t*>( ptr );
    const char32_t* charsEnd = charsBegin + header.count;
    const fnta::Glyph* glyphsBegin = reinterpret_cast<const fnta::Glyph*>( charsEnd );
    const fnta::Glyph* glyphsEnd = glyphsBegin + header.count;

    std::span<const char32_t> charSpan{ charsBegin, charsEnd };
    std::span<const fnta::Glyph> glyphSpan{ glyphsBegin, glyphsEnd };
    m_width = header.width;
    m_height = header.height;
    m_lineHeight = header.lineHeight;
    m_glyphMap = GlyphMap{ charSpan, glyphSpan };

}

float Font::height() const
{
    return static_cast<float>( m_lineHeight ) * m_scale;
}

float Font::textLength( std::u32string_view text ) const
{
    float ret = 0.0f;
    const auto sum = [this]( auto ch ) -> float
    {
        const fnta::Glyph* glyph = m_glyphMap.find( ch );
        assert( glyph );
        return static_cast<float>( glyph->advance[ 0 ] ) * m_scale;
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
        const fnta::Glyph* glyph = m_glyphMap.find( chr );
        assert( glyph );
        auto& sprite = pushConstant.m_sprites[ i ];
        std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph, extent, surfacePos, m_scale );
        surfacePos.x += static_cast<float>( glyph->advance[ 0 ] ) * m_scale;
    }
    return { pushData, pushConstant };
}

}
