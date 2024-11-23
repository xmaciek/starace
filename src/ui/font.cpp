#include <ui/font.hpp>

#include <ui/input.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>
#include <ui/remapper.hpp>

#include <renderer/renderer.hpp>

#include <profiler.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <memory_resource>
#include <vector>
#include <tuple>

static std::tuple<math::vec4, math::vec4> composeSprite( const fnta::Glyph& glyph, math::vec2 extent, math::vec2 cursor, float lineHeightMismatch, float scale )
{
    math::vec2 position{ glyph.position[ 0 ], glyph.position[ 1 ] };
    math::vec2 padding = math::vec2{ glyph.padding[ 0 ], glyph.padding[ 1 ] } * lineHeightMismatch * scale;
    math::vec2 size{ glyph.size[ 0 ], glyph.size[ 1 ] };
    math::vec2 uv1 = position / extent;
    math::vec2 uv2 = size / extent;

    size *= lineHeightMismatch * scale;
    math::vec2 topLeft = cursor + padding;

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

Font::RenderText Font::composeText( const math::vec4& color, std::u32string_view text, const math::vec2& geometry ) const
{
    ZoneScoped;
    assert( text.size() < ui::PushConstant<ui::Pipeline::eSpriteSequence>::INSTANCES );

    RenderText ret{
        .pushData{
            .m_pipeline = g_uiProperty.pipelineSpriteSequence(),
            .m_verticeCount = 6,
            .m_instanceCount = 0,
        },
        .pushConstant{
            .m_color = color,
        },
    };
    if ( text.empty() ) [[unlikely]] return ret;

    math::vec2 cursor{};
    uint32_t lastBreakPosition = 0;
    uint32_t lastBreakCursorPos = 0;
    uint32_t lastInstanceCount = 0;

    auto breakLine = [&ret, &cursor, lineHeight = (float)m_lineHeight * m_scale]()
    {
        ret.extent.x = std::max( ret.extent.x, cursor.x );
        cursor.x = 0.0f;
        cursor.y += lineHeight;
    };

    enum Charset {
        eUnknown,
        eLatin,
        eHiragana,
        eKatakana,
        eCJK,
        ePUA,
    };
    auto charset = []( char32_t chr )
    {
        if ( chr < 0x180 ) [[likely]] return Charset::eLatin;
        if ( chr == 0 ) return Charset::eUnknown;
        if ( chr >= 0xE000 && chr < 0xF900 ) return Charset::ePUA;
        if ( chr >= 0x3041 && chr < 0x30A0 ) return Charset::eHiragana;
        if ( chr >= 0x30A0 && chr < 0x3100 ) return Charset::eKatakana;
        if ( chr >= 0x31F0 && chr < 0x3200 ) return Charset::eKatakana;
        if ( chr >= 0x4E00 && chr < 0xA000 ) return Charset::eCJK;
        return Charset::eUnknown;
    };

    auto canWordWrap = [lastChrset = Charset::eLatin, charset]( char32_t chr ) mutable -> bool
    {
        const auto chrset = charset( chr );
        if ( chrset != lastChrset ) [[unlikely]] {
            lastChrset = chrset;
            return true;
        }
        if ( chrset == Charset::eCJK ) [[unlikely]] return true;

        switch ( chr ) {
        [[likely]] case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '+':
        case '-':
        case '/':
        case '\\':
            return true;
        }
        return false;
    };

    for ( uint32_t i = 0; i < text.size(); ++i ) {
        char32_t chr = text[ i ];
        if ( canWordWrap( chr ) ) [[unlikely]] {
            lastBreakCursorPos = (uint32_t)cursor.x;
            lastBreakPosition = i;
            lastInstanceCount = ret.pushData.m_instanceCount;
        }
        switch ( chr ) {
        case '\n':
            breakLine();
            continue;
        case ' ':
        case '\t': {
            const auto [ glyph, _, _2, _3 ] = getGlyph( ' ' );
            const float spaceWidth = glyph.advance[ 0 ] * m_scale;
            if ( chr == ' ' ) [[likely]] {
                cursor.x += spaceWidth;
            }
            else {
                const float tabWidth = 4.0f * spaceWidth;
                cursor.x = ( std::floor( cursor.x / tabWidth ) + 1.0f ) * tabWidth;
            }
        } continue;
        [[likely]] default: break;
        }

        if ( chr < (char32_t)Action::Enum::base || chr >= (char32_t)Action::Enum::end ) [[likely]] {
            appendRenderText( cursor, ret.pushData, ret.pushConstant, chr );
        }
        else {
            std::array<char32_t, 20> remapped;
            uint32_t remappedCount = m_remapper->apply( chr, remapped );
            for ( uint32_t i = 0; i < remappedCount; ++i ) {
                appendRenderText( cursor, ret.pushData, ret.pushConstant, remapped[ i ] );
            }
        }

        if ( ( cursor.x > geometry.x ) && ( lastBreakCursorPos != 0 ) ) [[unlikely]] {
            ret.pushData.m_instanceCount = lastInstanceCount;
            i = lastBreakPosition - 1;
            lastBreakCursorPos = 0;
            breakLine();
        }
    }
    ret.extent.x = std::max( ret.extent.x, cursor.x );
    ret.extent.y = cursor.y + (float)m_lineHeight * m_scale;
    return ret;
}

void Font::appendRenderText( math::vec2& cursor, PushData& pushData, ui::PushConstant<ui::Pipeline::eSpriteSequence>& pushConstant, char32_t chr ) const
{
    assert( pushData.m_instanceCount < pushConstant.INSTANCES );
    const auto [ glyph, texture, size, lineHeight ] = getGlyph( chr );
    const float lineHeightMismatch = ( lineHeight && lineHeight != m_lineHeight ) ? (float)m_lineHeight / (float)lineHeight : 1.0f;

    if ( !glyph ) [[unlikely]] {
        cursor.x += static_cast<float>( glyph.advance[ 0 ] ) * lineHeightMismatch * m_scale;
        return;
    };

    auto& sprite = pushConstant.m_sprites[ pushData.m_instanceCount++ ];
    std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph, size, cursor, lineHeightMismatch, m_scale );
    cursor.x += static_cast<float>( glyph.advance[ 0 ] ) * lineHeightMismatch * m_scale;
    auto it = std::find_if( pushData.m_fragmentTexture.begin() + 1, pushData.m_fragmentTexture.end(), [texture]( const auto& r ){ return r == texture; } );
    if ( it == pushData.m_fragmentTexture.end() ) [[unlikely]] {
        it = std::find_if( pushData.m_fragmentTexture.begin() + 1, pushData.m_fragmentTexture.end(), []( const auto& r ) { return r == 0; } );
    }
    sprite.m_whichAtlas = (uint32_t)std::distance( pushData.m_fragmentTexture.begin() + 1, it );
    pushData.m_fragmentTexture[ 1 + sprite.m_whichAtlas ] = texture;
    return;
}

std::tuple<Font::Glyph, Texture, math::vec2, uint32_t> Font::getGlyph( char32_t ch ) const
{
    const Glyph* g = m_glyphMap.find( ch );
    if ( g ) [[likely]] return std::make_tuple( *g, m_texture, extent(), m_lineHeight );
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
