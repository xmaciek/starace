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

constexpr static float FONT_SIZE_SCALE = 2.0f;
constexpr static float FONT_RESOLUTION_SCALE = 64.0f * FONT_SIZE_SCALE;
constexpr static uint32_t TILE_COUNT = 10;
constexpr static uint32_t TILE_PADDING = 2;

static std::array<math::vec4, 6> composeVertice( math::vec4 uv, math::vec2 advance, math::vec2 size, math::vec2 padding, math::vec2 originPointHack )
{
    padding *= math::vec2{ 1.0f, -1.0f };
    padding += originPointHack; // top vs bottom
    const math::vec2 topLeft = advance + padding;
    const math::vec2 botLeft = advance + padding + math::vec2{ 0.0f, size.y };
    const math::vec2 topRight = advance + padding + math::vec2{ size.x, 0.0f };
    const math::vec2 botRight = advance + padding + size;

    const math::vec2 uvTopLeft{ uv.x, uv.y };
    const math::vec2 uvBotLeft{ uv.x, uv.w };
    const math::vec2 uvTopRight{ uv.z, uv.y };
    const math::vec2 uvBotRight{ uv.z, uv.w };
    return {
        math::vec4{ topLeft, uvTopLeft },
        math::vec4{ botLeft, uvBotLeft },
        math::vec4{ botRight, uvBotRight },

        math::vec4{ botRight, uvBotRight },
        math::vec4{ topRight, uvTopRight},
        math::vec4{ topLeft, uvTopLeft },
    };
}

template <size_t TPitch>
constexpr std::pair<size_t, size_t> indexToXY( size_t i ) noexcept
{
    return {
        i % TPitch,
        i / TPitch,
    };
}

template <size_t TPitch>
math::vec4 makeSlotUV( size_t i, math::vec2 renornalizedUV ) noexcept
{
    constexpr float advance = 1.0f / TPitch;
    const auto [ x, y ] = indexToXY<TPitch>( i );
    return {
        advance * x,
        advance * y,
        advance * x + renornalizedUV.x,
        advance * y + renornalizedUV.y,
    };
}

struct BlitIterator {
    using value_type = uint8_t;
    using size_type = size_t;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    pointer m_data = nullptr;
    const pointer m_end = nullptr;
    size_type m_dstPitch = 0;
    size_type m_dstX = 0;
    size_type m_dstY = 0;
    size_type m_srcWidth = 0;
    size_type m_i = 0;

    BlitIterator( pointer p, const pointer end, size_type dstPitch, size_type dstX, size_type dstY, size_type srcWidth ) noexcept
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
        const size_t begin = m_dstPitch * m_dstY + m_dstX;
        const size_t offset = m_dstPitch * ( m_i / m_srcWidth ) + ( m_i % m_srcWidth );
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

static std::tuple<Font::Glyph, uint32_t, std::pmr::vector<uint8_t>> makeGlyph( const FT_Face& face, char32_t ch, uint32_t index, uint32_t dstPitch )
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
    std::pmr::vector<uint8_t> data{};
    data.resize( slot->bitmap.pitch * slot->bitmap.rows );
    std::copy_n( slot->bitmap.buffer, data.size(), data.begin() );

    glyph.advance = math::vec2{ slot->metrics.horiAdvance, slot->metrics.vertAdvance } / FONT_RESOLUTION_SCALE;
    glyph.padding = math::vec2{ slot->metrics.horiBearingX, slot->metrics.horiBearingY } / FONT_RESOLUTION_SCALE;
    glyph.size = math::vec2{ slot->metrics.width, slot->metrics.height } / FONT_RESOLUTION_SCALE;
    const math::vec2 renormalizedUV = glyph.size / (float)dstPitch * FONT_SIZE_SCALE;
    glyph.uv = makeSlotUV<TILE_COUNT>( index, renormalizedUV );
    assert( glyph.uv.x >= 0.0f );
    assert( glyph.uv.x <= 1.0f );
    assert( glyph.uv.y >= 0.0f );
    assert( glyph.uv.y <= 1.0f );
    return { glyph, slot->bitmap.pitch, data };
}

Font::Font( const CreateInfo& fontInfo, uint32_t height )
: m_height( height )
{
    ZoneScoped;
    assert( fontInfo.fontFileContent );
    assert( !fontInfo.fontFileContent->empty() );
    assert( fontInfo.renderer );
    assert( !fontInfo.charset.empty() );
    assert( fontInfo.charset.size() <= TILE_COUNT * TILE_COUNT );
    assert( std::is_sorted( fontInfo.charset.begin(), fontInfo.charset.end() ) );
    assert( height > 0 );

    FT_Library library{};
    [[maybe_unused]]
    const FT_Error freetypeInitErr = FT_Init_FreeType( &library );
    assert( !freetypeInitErr );

    const FT_Open_Args openArgs{
        .flags = FT_OPEN_MEMORY,
        .memory_base = reinterpret_cast<const FT_Byte*>( fontInfo.fontFileContent->data() ),
        .memory_size = static_cast<FT_Long>( fontInfo.fontFileContent->size() ),
    };

    FT_Face face{};
    [[maybe_unused]]
    const FT_Error newFaceErr = FT_Open_Face( library, &openArgs, 0, &face );
    assert( !newFaceErr );

    const FT_UInt pixelSize = static_cast<FT_UInt>( FONT_SIZE_SCALE * (float)height );
    [[maybe_unused]]
    const FT_Error pixelSizeErr = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    assert( !pixelSizeErr );

    const size_t textureSize = std::pow( FONT_SIZE_SCALE * ( height + TILE_PADDING ) * TILE_COUNT, 2.0f );
    std::pmr::vector<uint8_t> texture( textureSize, fontInfo.renderer->allocator() );
    const size_t dstPitch = static_cast<size_t>( FONT_SIZE_SCALE * (float)( height + TILE_PADDING ) * TILE_COUNT );

    for ( size_t i = 0; i < fontInfo.charset.size(); i++ ) {
        auto [ glyph, pitch, data ] = makeGlyph( face, fontInfo.charset[ i ], i, dstPitch );
        m_glyphs.pushBack( fontInfo.charset[ i ], glyph );
        if ( pitch == 0 ) {
            continue;
        }
        auto [ x, y ] = indexToXY<TILE_COUNT>( i );
        x *= dstPitch / TILE_COUNT;
        y *= dstPitch / TILE_COUNT;
        BlitIterator dst{ texture.data(), texture.data() + texture.size(), dstPitch, x, y, pitch };
        std::copy( data.begin(), data.end(), dst );
    }

    FT_Done_FreeType( library );

    const TextureCreateInfo tci{
        .width = static_cast<uint16_t>( dstPitch ),
        .height = static_cast<uint16_t>( dstPitch ),
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
    assert( text.size() < PushConstant<Pipeline::eShortString>::charCount );
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eShortString ),
        .m_verticeCount = static_cast<uint32_t>( text.size() * 6 ),
        .m_texture = m_texture,
    };

    PushConstant<Pipeline::eShortString> pushConstant{};
    pushConstant.m_color = color;
    auto verticeIt = pushConstant.m_vertices.begin();
    math::vec2 advance{};

    for ( auto ch : text ) {
        const Glyph* glyph = m_glyphs[ ch ];
        assert( glyph );
        const auto vertice = composeVertice( glyph->uv, advance, glyph->size, glyph->padding, { 0.0f, m_height } );
        std::copy_n( vertice.begin(), vertice.size(), verticeIt );
        std::advance( verticeIt, vertice.size() );
        advance.x += glyph->advance.x;
    }
    return { pushBuffer, pushConstant };
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
