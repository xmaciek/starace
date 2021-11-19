#include "font.hpp"

#include "texture.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/gtc/matrix_transform.hpp>
#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>

constexpr static float FONT_SIZE_SCALE = 2.0f;
constexpr static float FONT_RESOLUTION_SCALE = 64.0f * FONT_SIZE_SCALE;

static std::array<glm::vec4, 6> composeUV( glm::vec4 vec )
{
    return {
        glm::vec4{ vec.x, vec.y, 0.0f, 0.0f },
        glm::vec4{ vec.x, vec.w, 0.0f, 0.0f },
        glm::vec4{ vec.z, vec.w, 0.0f, 0.0f },
        glm::vec4{ vec.z, vec.w, 0.0f, 0.0f },
        glm::vec4{ vec.z, vec.y, 0.0f, 0.0f },
        glm::vec4{ vec.x, vec.y, 0.0f, 0.0f },
    };
}

static std::array<glm::vec4, 6> composeVertice( glm::vec2 advance, glm::vec2 size, glm::vec2 padding )
{
    return {
        glm::vec4{ advance.x + padding.x, -padding.y, 0.0f, 0.0f },
        glm::vec4{ advance.x + padding.x, size.y - padding.y, 0.0f, 0.0f },
        glm::vec4{ advance.x + padding.x + size.x, size.y - padding.y, 0.0f, 0.0f },

        glm::vec4{ advance.x + padding.x + size.x, size.y - padding.y, 0.0f, 0.0f },
        glm::vec4{ advance.x + padding.x + size.x, -padding.y, 0.0f, 0.0f },
        glm::vec4{ advance.x + padding.x, -padding.y, 0.0f, 0.0f },
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
glm::vec4 makeSlotUV( size_t i, glm::vec2 normUV ) noexcept
{
    constexpr float advance = 1.0f / TPitch;
    const auto [ x, y ] = indexToXY<TPitch>( i );
    return {
        advance * x,
        advance * y,
        advance * x + advance * normUV.x,
        advance * y + advance * normUV.y,
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
    size_type m_dstPitch = 0;
    size_type m_dstX = 0;
    size_type m_dstY = 0;
    size_type m_srcWidth = 0;
    size_type m_i = 0;

    BlitIterator( pointer p, size_type dstPitch, size_type dstX, size_type dstY, size_type srcWidth ) noexcept
    : m_data{ p }
    , m_dstPitch{ dstPitch }
    , m_dstX{ dstX }
    , m_dstY{ dstY }
    , m_srcWidth{ srcWidth }
    {}

    reference operator * () const
    {
        assert( m_data );
        assert( m_dstPitch );
        assert( m_srcWidth );
        const size_t begin = m_dstPitch * m_dstY + m_dstX;
        const size_t offset = m_dstPitch * ( m_i / m_srcWidth ) + ( m_i % m_srcWidth );
        return *( m_data + begin + offset );
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

static std::tuple<Font::Glyph, uint32_t, std::pmr::vector<uint8_t>> makeGlyph( const FT_Face& face, char32_t ch, uint32_t index, uint32_t pixelSize )
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

    glyph.advance = glm::vec2{ slot->metrics.horiAdvance, slot->metrics.vertAdvance } / FONT_RESOLUTION_SCALE;
    glyph.padding = glm::vec2{ slot->metrics.horiBearingX, slot->metrics.horiBearingY } / FONT_RESOLUTION_SCALE;
    glyph.size = glm::vec2{ slot->metrics.width, slot->metrics.height } / FONT_RESOLUTION_SCALE;
    glyph.uv = makeSlotUV<12>( index, glyph.size / (float)pixelSize * FONT_SIZE_SCALE );
    return { glyph, slot->bitmap.pitch, data };
}

Font::Font( std::string_view fontname, uint32_t h )
: m_name( fontname )
, m_height( h )
{
    ZoneScoped;
    FT_Library library{};
    [[maybe_unused]]
    const FT_Error freetypeInitErr = FT_Init_FreeType( &library );
    assert( !freetypeInitErr );

    FT_Face face{};
    [[maybe_unused]]
    const FT_Error newFaceErr = FT_New_Face( library, fontname.data(), 0, &face );
    assert( !newFaceErr );

    const FT_UInt pixelSize = static_cast<FT_UInt>( FONT_SIZE_SCALE * (float)h );
    [[maybe_unused]]
    const FT_Error pixelSizeErr = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    assert( !pixelSizeErr );

    const size_t textureSize = ( FONT_SIZE_SCALE * h * 12 ) * ( FONT_SIZE_SCALE * h * 12);
    Renderer* renderer = Renderer::instance();
    assert( renderer );
    std::pmr::vector<uint8_t> texture( textureSize, renderer->allocator() );
    const size_t dstPitch = static_cast<size_t>( FONT_SIZE_SCALE * (float)h * 12 );

    char32_t chars[] = U"0123456789"
        U"abcdefghijklmnopqrstuvwxyz"
        U"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        U" `~',./\\?+-*!@#$%^&()[]{};:<>";
    std::sort( std::begin( chars ), std::end( chars ) - 1 );

    for ( size_t i = 0; i < std::size( chars ) - 1; i++ ) {
        auto [ glyph, pitch, data ] = makeGlyph( face, chars[ i ], i, pixelSize );
        m_glyphs.pushBack( chars[ i ], glyph );
        if ( pitch == 0 ) {
            continue;
        }
        auto [ x, y ] = indexToXY<12>( i );
        x *= dstPitch / 12;
        y *= dstPitch / 12;
        BlitIterator dst{ texture.data(), dstPitch, x, y, pitch };
        std::copy( data.begin(), data.end(), dst );
    }

    FT_Done_FreeType( library );

    m_texture = renderer->createTexture(
        dstPitch
        , dstPitch
        , TextureFormat::eR
        , false
        , std::move( texture )
    );
}

Font::~Font()
{
    destroyTexture( m_texture );
}

uint32_t Font::height() const
{
    return m_height;
}

uint32_t Font::textLength( std::u32string_view text )
{
    float length = 0;
    for ( auto ch : text ) {
        Glyph* glyph = m_glyphs[ ch ];
        assert( glyph );
        length += glyph->advance.x;
    }
    return static_cast<uint32_t>( length );
}

Font::RenderText Font::composeText( const glm::vec4& color, std::u32string_view text )
{
    ZoneScoped;
    assert( text.size() < PushConstant<Pipeline::eShortString>::charCount );
    PushBuffer<Pipeline::eShortString> pushBuffer{};
    pushBuffer.m_texture = m_texture;
    pushBuffer.m_verticeCount = text.size() * 6;

    PushConstant<Pipeline::eShortString> pushConstant{};
    pushConstant.m_color = color;
    auto verticeIt = pushConstant.m_vertices.begin();
    auto uvIt = pushConstant.m_uv.begin();
    glm::vec2 advance{};

    for ( auto ch : text ) {
        const Glyph* glyph = m_glyphs[ ch ];
        assert( glyph );
        const auto vertice = composeVertice( advance, glyph->size, glyph->padding );
        const auto uv = composeUV( glyph->uv );
        std::copy_n( vertice.begin(), vertice.size(), verticeIt );
        std::copy_n( uv.begin(), uv.size(), uvIt );
        std::advance( verticeIt, vertice.size() );
        std::advance( uvIt, uv.size() );
        advance += glyph->advance;
    }
    return { pushBuffer, pushConstant };
}

void Font::renderText( RenderContext rctx, const glm::vec4& color, double x, double y, std::u32string_view text )
{
    assert( !text.empty() );
    rctx.model = glm::translate( rctx.model, glm::vec3{ x, y, 0.0 } );

    auto [ pushBuffer, pushConstant ] = composeText( color, text );
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    rctx.renderer->push( &pushBuffer, &pushConstant );
}
