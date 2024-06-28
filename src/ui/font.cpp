#include <ui/font.hpp>

#include <ui/input.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>
#include <ui/remapper.hpp>

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <memory_resource>
#include <vector>
#include <tuple>

static std::tuple<math::vec4, math::vec4> composeSprite( const fnta::Glyph& glyph, math::vec2 extent, math::vec2 surfacePos, float lineHeightMismatch, float scale )
{
    math::vec2 position{ glyph.position[ 0 ], glyph.position[ 1 ] };
    math::vec2 padding = math::vec2{ glyph.padding[ 0 ], glyph.padding[ 1 ] } * lineHeightMismatch * scale;
    math::vec2 size{ glyph.size[ 0 ], glyph.size[ 1 ] };
    math::vec2 uv1 = position / extent;
    math::vec2 uv2 = size / extent;

    size *= lineHeightMismatch * scale;
    math::vec2 topLeft = surfacePos + padding;

    return std::make_tuple(
        math::vec4{ topLeft.x, topLeft.y, size.x, size.y },
        math::vec4{ uv1.x, uv1.y, uv2.x, uv2.y }
    );
}

namespace ui {

Font::Font( const CreateInfo& ci )
: m_upstream{ ci.upstream }
, m_remapper{ ci.remapper }
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
    std::array<char32_t, 20> remapped;
    uint32_t remappedCount = 0;
    for ( char32_t chr : text ) {
        switch ( chr ) {
        case '\n':
            retY += height();
            retNext = std::max( ret, retNext );
            ret = 0.0f;
            break;
        case '\t': {
            auto [ glyph, _, _2, _3 ] = getGlyph( ' ' );
            assert( glyph );
            const float tabWidth = 4.0f * glyph.advance[ 0 ] * m_scale;
            ret = ( std::floor( ret / tabWidth ) + 1.0f ) * tabWidth;
        } break;
        [[likely]] default:
            if ( chr < (char32_t)Action::Enum::base || chr >= (char32_t)Action::Enum::end ) [[likely]] {
                auto [ glyph, _, _2, lineHeight ] = getGlyph( chr );
                const float lineHeightMismatch = ( lineHeight && lineHeight != m_lineHeight )
                    ? (float)m_lineHeight / (float)lineHeight : 1.0f;
                ret += glyph.advance[ 0 ] * lineHeightMismatch * m_scale;
                continue;
            }
            remappedCount = m_remapper->apply( chr, remapped );
            for ( uint32_t i = 0; i < remappedCount; ++i ) {
                auto [ glyph, _, _2, lineHeight ] = getGlyph( remapped[ i ] );
                const float lineHeightMismatch = ( lineHeight && lineHeight != m_lineHeight )
                    ? (float)m_lineHeight / (float)lineHeight : 1.0f;
                ret += glyph.advance[ 0 ] * lineHeightMismatch * m_scale;
            }
            break;
        }
    }
    return { std::max( ret, retNext ), retY };
}

Font::RenderText Font::composeText( const math::vec4& color, std::u32string_view text ) const
{
    ZoneScoped;
    assert( text.size() < ui::PushConstant<ui::Pipeline::eSpriteSequence>::INSTANCES );

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
        .m_verticeCount = 6,
        .m_instanceCount = 0,
    };

    ui::PushConstant<ui::Pipeline::eSpriteSequence> pushConstant{};
    pushConstant.m_color = color;
    math::vec2 surfacePos{};
    math::vec2 extent{ m_width, m_height };
    std::array<char32_t, 20> remapped;
    uint32_t remappedCount = 0;

    for ( char32_t chr : text ) {
        switch ( chr ) {
        case '\n':
            surfacePos.x = 0.0f;
            surfacePos.y += (float)m_lineHeight;
            continue;
        case '\t': {
            const auto [ glyph, _, _2, _3 ] = getGlyph( ' ' );
            assert( glyph );
            const float tabWidth = 4.0f * glyph.advance[ 0 ] * m_scale;
            surfacePos.x = ( std::floor( surfacePos.x / tabWidth ) + 1.0f ) * tabWidth;
        } continue;
        [[likely]] default:
            if ( chr < (char32_t)Action::Enum::base || chr >= (char32_t)Action::Enum::end ) [[likely]] {
                appendRenderText( surfacePos, pushData, pushConstant, chr );
                continue;
            }
            remappedCount = m_remapper->apply( chr, remapped );
            for ( uint32_t i = 0; i < remappedCount; ++i ) {
                appendRenderText( surfacePos, pushData, pushConstant, remapped[ i ] );
            }
            break;
        }
    }
    return { pushData, pushConstant };
}

void Font::appendRenderText( math::vec2& surfacePos, PushData& pushData, ui::PushConstant<ui::Pipeline::eSpriteSequence>& pushConstant, char32_t chr ) const
{
    assert( pushData.m_instanceCount < pushConstant.INSTANCES );
    const auto [ glyph, texture, size, lineHeight ] = getGlyph( chr );
    const float lineHeightMismatch = ( lineHeight && lineHeight != m_lineHeight ) ? (float)m_lineHeight / (float)lineHeight : 1.0f;

    if ( !glyph ) [[unlikely]] {
        surfacePos.x += static_cast<float>( glyph.advance[ 0 ] ) * lineHeightMismatch * m_scale;
        return;
    };

    auto& sprite = pushConstant.m_sprites[ pushData.m_instanceCount++ ];
    std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph, size, surfacePos, lineHeightMismatch, m_scale );
    surfacePos.x += static_cast<float>( glyph.advance[ 0 ] ) * lineHeightMismatch * m_scale;
    auto it = std::find_if( pushData.m_fragmentTexture.begin() + 1, pushData.m_fragmentTexture.end(), [texture]( const auto& r ){ return r == texture; } );
    if ( it == pushData.m_fragmentTexture.end() ) {
        it = std::find_if( pushData.m_fragmentTexture.begin() + 1, pushData.m_fragmentTexture.end(), []( const auto& r ) { return r == 0; } );
    }
    sprite.m_whichAtlas = (uint32_t)std::distance( pushData.m_fragmentTexture.begin() + 1, it );
    pushData.m_fragmentTexture[ 1 + sprite.m_whichAtlas ] = texture;
    return;
}

std::tuple<Font::Glyph, Texture, math::vec2, uint32_t> Font::getGlyph( char32_t ch ) const
{
    const Glyph* g = m_glyphMap.find( ch );
    if ( g ) return std::make_tuple( *g, m_texture, extent(), m_lineHeight );
    if ( !m_upstream ) return {};
    return m_upstream->getGlyph( ch );
}

math::vec2 Font::extent() const
{
    return math::vec2{ m_width ? m_width : 1, m_height ? m_height : 1 };
}


Font::Sprite Font::operator [] ( Hash::value_type h ) const
{
    auto [ g, _, _2, _3 ] = getGlyph( h );
    return Sprite{ g.position[ 0 ], g.position[ 1 ], g.size[ 0 ], g.size[ 1 ] };
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
