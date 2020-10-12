#include "font.hpp"

#include "texture.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>

constexpr static float FONT_SIZE_SCALE = 2.0f;
constexpr static float FONT_RESOLUTION_SCALE = 64.0f * FONT_SIZE_SCALE;

static Font::Glyph makeDlist( const FT_Face& face, char32_t ch )
{
    const FT_UInt glyphIndex = FT_Get_Char_Index( face, ch );
    if ( const FT_Error error = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT ); error ) {
        std::cout << "Failed to load glyph for codepoint: " << std::hex << ch << std::endl;
        return {};
    }

    FT_GlyphSlot slot = face->glyph;
    if ( const FT_Error error = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL ); error ) {
        std::cout << "Failed to render glyph in lcd mode" << std::endl;
        return {};
    }
    assert( slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY );

    const size_t size = slot->bitmap.pitch * slot->bitmap.rows;
    std::vector<uint32_t> data( size );

    std::transform( slot->bitmap.buffer, slot->bitmap.buffer + size, data.begin(),
        []( unsigned char c ) {
            uint32_t ret = 0;
            ret = c;
            ret <<= 8;
            ret |= c;
            ret <<= 8;
            ret |= c;
            ret <<= 8;
            ret |= c;
            return ret;
        }
    );
    Font::Glyph glyph{};
    glyph.texture = Renderer::instance()->createTexture(
        slot->bitmap.pitch
        , slot->bitmap.rows
        , Texture::Format::eRGBA
        , false
        , reinterpret_cast<const uint8_t*>( data.data() )
    );

    glyph.advance = glm::vec2{ slot->metrics.horiAdvance, slot->metrics.vertAdvance } / FONT_RESOLUTION_SCALE;
    glyph.bearing = glm::vec2{ slot->metrics.horiBearingX, slot->metrics.horiBearingY } / FONT_RESOLUTION_SCALE;
    glyph.size = glm::vec2{ slot->metrics.width, slot->metrics.height } / FONT_RESOLUTION_SCALE;
    return glyph;
}

Font::Font( std::string_view fontname, uint32_t h )
: m_name( fontname )
, m_glyphs( 128 )
, m_height( h )
{
    FT_Library library{};
    if ( const FT_Error error = FT_Init_FreeType( &library ); error ) {
        std::cout << "Unable to initialize freetype " << std::endl;
        return;
    }

    FT_Face face{};
    if ( const FT_Error error = FT_New_Face( library, fontname.data(), 0, &face ); error ) {
        std::cout << "Unable to load font file: " << fontname << std::endl;
        FT_Done_FreeType( library );
        return;
    }

    if ( const FT_Error error = FT_Set_Pixel_Sizes( face, 0, h * FONT_SIZE_SCALE ); error ) {
        std::cout << "Failed to set font size" << std::endl;
        FT_Done_FreeType( library );
        return;
    }

    for ( char16_t i = 0; i < 128; i++ ) {
        m_glyphs[ i ] = makeDlist( face, i );
    }

    FT_Done_FreeType( library );
}

Font::~Font()
{
    for ( const auto& it : m_glyphs ) {
        destroyTexture( it.texture );
    }
}

uint32_t Font::textLength( std::string_view text )
{
    uint32_t length = 0;
    for ( char i : text ) {
        length += m_glyphs[ static_cast<size_t>( i ) ].advance.x;
    }
    return length;
}

uint32_t Font::height() const
{
    return m_height;
}

void Font::renderText( RenderContext rctx, const glm::vec4& color, double x, double y, std::string_view text )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ x, y, 0.0 } );
    for ( char ch : text ) {
        const Glyph& glyph = m_glyphs[ ch ];
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
        pushBuffer.m_texture = glyph.texture;
        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_color = color;
        pushConstant.m_vertices[ 0 ] = glm::vec4{ glyph.bearing, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ glyph.bearing.x, glyph.bearing.y - glyph.size.y, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ glyph.bearing.x + glyph.size.x, glyph.bearing.y - glyph.size.y, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ glyph.bearing.x + glyph.size.x, glyph.bearing.y, 0.0f, 0.0f };
        pushConstant.m_uv[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_uv[ 1 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 2 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 3 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };
        rctx.renderer->push( &pushBuffer, &pushConstant );
        rctx.model = glm::translate( rctx.model, glm::vec3{ glyph.advance.x, 0.0f, 0.0f } );
    }
}
