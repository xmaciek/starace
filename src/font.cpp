#include "font.hpp"

#include "texture.hpp"

#include <renderer/renderer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <span>

constexpr static float FONT_SIZE_SCALE = 2.0f;
constexpr static float FONT_RESOLUTION_SCALE = 64.0f * FONT_SIZE_SCALE;
constexpr static uint32_t TILE_COUNT = 10;
constexpr static uint32_t TILE_PADDING = 2;

static std::tuple<math::vec4, math::vec4> composeSprite( math::vec4 uv, math::vec2 advance, math::vec2 size, math::vec2 padding, math::vec2 originPointHack )
{
    padding *= math::vec2{ 1.0f, -1.0f };
    padding += originPointHack; // top vs bottom
    const math::vec2 topLeft = advance + padding;
    return std::make_tuple(
        math::vec4{ topLeft.x, topLeft.y, size.x, size.y },
        math::vec4{ uv.x, uv.y, uv.z - uv.x, uv.w - uv.y }
    );
}

struct BlitIterator {
    using value_type = uint8_t;
    using size_type = uint32_t;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    pointer m_data = nullptr;
    pointer m_end = nullptr;
    size_type m_dstPitch = 0;
    size_type m_dstX = 0;
    size_type m_dstY = 0;
    size_type m_srcWidth = 0;
    size_type m_i = 0;

    BlitIterator() noexcept = default;
    BlitIterator( pointer p, pointer end, size_type dstPitch, size_type dstX, size_type dstY, size_type srcWidth ) noexcept
    : m_data{ p }
    , m_end{ end }
    , m_dstPitch{ dstPitch }
    , m_dstX{ dstX }
    , m_dstY{ dstY }
    , m_srcWidth{ srcWidth }
    {
        assert( p < end );
    }

    reference operator * () const
    {
        assert( m_data );
        assert( m_dstPitch );
        assert( m_srcWidth );
        const difference_type begin = m_dstPitch * m_dstY + m_dstX;
        const difference_type offset = m_dstPitch * ( m_i / m_srcWidth ) + ( m_i % m_srcWidth );
        pointer address = m_data + begin + offset;
        assert( address < m_end );
        return *address;
    }

    BlitIterator& operator ++ ()
    {
        ++m_i;
        return *this;
    }
    BlitIterator& operator -- ()
    {
        --m_i;
        return *this;
    }
    BlitIterator operator ++ ( int )
    {
        auto ret = *this;
        ++m_i;
        return ret;
    }
    BlitIterator operator -- ( int )
    {
        auto ret = *this;
        --m_i;
        return ret;
    }
};

struct Atlas {
    using value_type = uint8_t;
    using pointer = value_type*;

    pointer m_begin = nullptr;
    pointer m_end = nullptr;
    uint32_t m_width = 0u;
    uint32_t m_height = 0u;
    uint32_t m_currentX = 0u;
    uint32_t m_currentY = 0u;
    uint32_t m_nextY = 0;

    std::tuple<uint32_t, uint32_t> coords( uint32_t width, uint32_t height, uint32_t padding )
    {
        assert( width <= m_width );
        assert( height <= m_height );
        assert( m_currentY + height < m_height );

        if ( ( m_currentX + width ) > m_width ) {
            m_currentX = 0u;
            m_currentY = m_nextY;
        }

        uint32_t retX = m_currentX;
        m_nextY = std::max( m_currentY + height + padding, m_nextY );
        m_currentX += width + padding;
        assert( m_currentY < m_height );
        return { retX, m_currentY };
    }

    std::tuple<BlitIterator, math::vec4> findPlace( uint32_t width, uint32_t height, uint32_t padding )
    {
        const auto [ x, y ] = coords( width, height, padding );
        const float fx = static_cast<float>( x ) / static_cast<float>( m_width );
        const float fy = static_cast<float>( y ) / static_cast<float>( m_height );
        math::vec4 xyxy{ fx, fy,
            fx + static_cast<float>( width ) / static_cast<float>( m_width ),
            fy + static_cast<float>( height ) / static_cast<float>( m_height ),
        };
        return {
            BlitIterator{ m_begin, m_end, m_width, x, y, width },
            xyxy,
        };

    }
};

static std::tuple<Font::Glyph, uint32_t, std::span<uint8_t>> makeGlyph( const FT_Face& face, char32_t ch )
{
    ZoneScoped;
    const FT_UInt glyphIndex = FT_Get_Char_Index( face, ch );

    [[maybe_unused]]
    const FT_Error loadOK = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT );
    assert( loadOK == 0 );

    FT_GlyphSlot slot = face->glyph;
    [[maybe_unused]]
    const FT_Error renderOK = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
    assert( renderOK == 0 );
    assert( slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY );

    Font::Glyph glyph{};
    uint8_t* data = reinterpret_cast<uint8_t*>( slot->bitmap.buffer );
    std::uintptr_t dataSize = static_cast<std::uintptr_t>( slot->bitmap.pitch ) * static_cast<std::uintptr_t>( slot->bitmap.rows );

    glyph.advance = math::vec2{ slot->metrics.horiAdvance, slot->metrics.vertAdvance } / FONT_RESOLUTION_SCALE;
    glyph.padding = math::vec2{ slot->metrics.horiBearingX, slot->metrics.horiBearingY } / FONT_RESOLUTION_SCALE;
    glyph.size = math::vec2{ slot->metrics.width, slot->metrics.height } / FONT_RESOLUTION_SCALE;
    return { glyph, slot->bitmap.pitch, std::span<uint8_t>( data, data + dataSize ) };
}

Font::Font( const CreateInfo& fontInfo, uint32_t height )
: m_height( height )
{
    ZoneScoped;
    assert( !fontInfo.fontFileContent.empty() );
    assert( fontInfo.renderer );
    assert( !fontInfo.charset.empty() );
    assert( std::is_sorted( fontInfo.charset.begin(), fontInfo.charset.end() ) );
    assert( height > 0 );

    FT_Library library{};
    [[maybe_unused]]
    const FT_Error freetypeInitErr = FT_Init_FreeType( &library );
    assert( !freetypeInitErr );

    const FT_Open_Args openArgs{
        .flags = FT_OPEN_MEMORY,
        .memory_base = reinterpret_cast<const FT_Byte*>( fontInfo.fontFileContent.data() ),
        .memory_size = static_cast<FT_Long>( fontInfo.fontFileContent.size() ),
    };

    FT_Face face{};
    [[maybe_unused]]
    const FT_Error newFaceErr = FT_Open_Face( library, &openArgs, 0, &face );
    assert( !newFaceErr );

    const FT_UInt pixelSize = static_cast<FT_UInt>( FONT_SIZE_SCALE * (float)height );
    [[maybe_unused]]
    const FT_Error pixelSizeErr = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    assert( !pixelSizeErr );

    const uint32_t dstPitch = static_cast<uint32_t>( FONT_SIZE_SCALE * (float)( height + TILE_PADDING ) * TILE_COUNT );
    const uint32_t textureSize = dstPitch * dstPitch;
    std::pmr::vector<uint8_t> texture( textureSize, fontInfo.renderer->allocator() );

    Atlas atlas {
        .m_begin = texture.data(),
        .m_end = texture.data() + texture.size(),
        .m_width = dstPitch,
        .m_height = dstPitch,
    };
    for ( char32_t ch : fontInfo.charset ) {
        auto [ glyph, pitch, data ] = makeGlyph( face, ch );
        BlitIterator dst;
        const math::vec2 size = glyph.size * FONT_SIZE_SCALE;
        std::tie( dst, glyph.uv ) = atlas.findPlace( static_cast<uint32_t>( size.x ), static_cast<uint32_t>( size.y ), TILE_PADDING );
        m_glyphs.pushBack( ch, glyph );
        if ( pitch == 0 ) {
            continue;
        }
        std::copy( data.begin(), data.end(), dst );
    }

    FT_Done_FreeType( library );

    const TextureCreateInfo tci{
        .width = static_cast<uint16_t>( dstPitch ),
        .height = static_cast<uint16_t>( dstPitch ),
        .mip0ByteCount = static_cast<uint32_t>( texture.size() ),
        .mips = 1,
        .format = TextureFormat::eR,
    };
    m_texture = fontInfo.renderer->createTexture( tci, std::move( texture ) );
}

Font::~Font()
{
    destroyTexture( m_texture );
}

uint32_t Font::height() const
{
    return m_height;
}

float Font::textLength( std::u32string_view text ) const
{
    float ret = 0.0f;
    const auto sum = [this]( auto ch ) -> float
    {
        const Glyph* glyph = m_glyphs[ ch ];
        assert( glyph );
        return glyph->advance.x;
    };
    for ( auto ch : text ) { ret += sum( ch ); }
    return ret;
}

Font::RenderText Font::composeText( const math::vec4& color, std::u32string_view text ) const
{
    ZoneScoped;
    assert( text.size() < PushConstant<Pipeline::eSpriteSequence>::INSTANCES );
    const uint32_t count = static_cast<uint32_t>( text.size() );

    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eSpriteSequence ],
        .m_verticeCount = 6,
        .m_instanceCount = count,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    PushConstant<Pipeline::eSpriteSequence> pushConstant{};
    pushConstant.m_color = color;
    math::vec2 advance{};

    for ( uint32_t i = 0; i < count; ++i ) {
        char32_t chr = text[ i ];
        const Glyph* glyph = m_glyphs[ chr ];
        assert( glyph );
        auto& sprite = pushConstant.m_sprites[ i ];
        std::tie( sprite.m_xywh, sprite.m_uvwh ) = composeSprite( glyph->uv, advance, glyph->size, glyph->padding, { 0.0f, m_height } );
        advance.x += glyph->advance.x;
    }
    return { pushData, pushConstant };
}

void Font::renderText( RenderContext rctx, const math::vec4& color, double x, double y, std::u32string_view text ) const
{
    assert( !text.empty() );
    rctx.model = math::translate( rctx.model, math::vec3{ x, y, 0.0 } );

    auto [ pushBuffer, pushConstant ] = composeText( color, text );
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    rctx.renderer->push( pushBuffer, &pushConstant );
}
