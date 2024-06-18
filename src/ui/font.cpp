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
: m_upstream{ ci.upstream }
, m_scale{ ci.scale }
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

Font::Font( Font&& rhs )
{
    std::swap( m_upstream, rhs.m_upstream );
    std::swap( m_width, rhs.m_width );
    std::swap( m_height, rhs.m_height );
    std::swap( m_lineHeight, rhs.m_lineHeight );
    std::swap( m_scale, rhs.m_scale );
    std::swap( m_texture, rhs.m_texture );
    std::swap( m_glyphMap, rhs.m_glyphMap );
}

Font& Font::operator = ( Font&& rhs )
{
    std::swap( m_upstream, rhs.m_upstream );
    std::swap( m_width, rhs.m_width );
    std::swap( m_height, rhs.m_height );
    std::swap( m_lineHeight, rhs.m_lineHeight );
    std::swap( m_scale, rhs.m_scale );
    std::swap( m_texture, rhs.m_texture );
    std::swap( m_glyphMap, rhs.m_glyphMap );
    return *this;
}


float Font::height() const
{
    return static_cast<float>( m_lineHeight ) * m_scale;
}

Texture Font::texture() const
{
    return m_texture;
}

math::vec2 Font::textGeometry( std::u32string_view text ) const
{
    float ret = 0.0f;
    float retNext = 0.0f;
    float retY = height();
    for ( auto chr : text ) {
        switch ( chr ) {
        case '\n':
            retY += height();
            retNext = std::max( ret, retNext );
            ret = 0.0f;
            break;
        case '\t': {
            const fnta::Glyph* glyph = m_glyphMap.find( ' ' );
            assert( glyph );
            const float tabWidth = 4.0f * glyph->advance[ 0 ] * m_scale;
            ret = ( std::floor( ret / tabWidth ) + 1.0f ) * tabWidth;
        } break;
        [[likely]] default:
            const fnta::Glyph* glyph = m_glyphMap.find( chr );
            assert( glyph );
            ret += static_cast<float>( glyph->advance[ 0 ] ) * m_scale;
        }
    }
    return { std::max( ret, retNext ), retY };
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
    pushData.m_resource[ 2 ].texture = m_texture;

    ui::PushConstant<ui::Pipeline::eSpriteSequence> pushConstant{};
    pushConstant.m_color = color;
    math::vec2 surfacePos{};
    math::vec2 extent{ m_width, m_height };

    for ( uint32_t i = 0; i < count; ++i ) {
        char32_t chr = text[ i ];
        switch ( chr ) {
        case '\n':
            surfacePos.x = 0.0f;
            surfacePos.y += (float)m_lineHeight;
            break;
        case '\t': {
            const fnta::Glyph* glyph = m_glyphMap.find( ' ' );
            assert( glyph );
            const float tabWidth = 4.0f * glyph->advance[ 0 ] * m_scale;
            surfacePos.x = ( std::floor( surfacePos.x / tabWidth ) + 1.0f ) * tabWidth;
        } break;
        [[likely]] default:
            const fnta::Glyph* glyph = m_glyphMap.find( chr );
            assert( glyph );
            auto& sprite = pushConstant.m_sprites[ i ];
            std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph, extent, surfacePos, m_scale );
            surfacePos.x += static_cast<float>( glyph->advance[ 0 ] ) * m_scale;
        }
    }
    return { pushData, pushConstant };
}

math::vec2 Font::extent() const
{
    return math::vec2{ m_width, m_height };
}


Font::Sprite Font::operator [] ( Hash::value_type h ) const
{
    const auto* ptr = m_glyphMap.find( h );
    if ( !ptr ) {
        return m_upstream ? (*m_upstream)[ h ] : Sprite{};
    }
    return Sprite{ ptr->position[ 0 ], ptr->position[ 1 ], ptr->size[ 0 ], ptr->size[ 1 ] };
}

Font::Sprite::operator math::vec4 () const noexcept
{
    return math::vec4{ x, y, w, h };
}

math::vec4 Font::Sprite::operator / ( const math::vec2& extent ) const noexcept
{
    return math::vec4{ x, y, w, h } / math::vec4{ extent.x, extent.y, extent.x, extent.y };
}

}
