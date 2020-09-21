#include "font.hpp"

#include "render_pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

Font::Font( const char* fontname, uint32_t h )
: m_name( fontname )
, m_charWidth( 128 )
, m_textures( 128 )
, m_charData( 128 )
, m_height( h )
{
    m_listBase = glGenLists( 128 );
    glGenTextures( 128, m_textures.data() );
    TTF_Font* font = TTF_OpenFont( fontname, h );

    for ( uint8_t i = 0; i < 128; i++ ) {
        makeDlist( font, i );
    }
    TTF_CloseFont( font );
}

Font::~Font()
{
    glDeleteLists( m_listBase, 128 );
    glDeleteTextures( 128, m_textures.data() );
}

static uint32_t pow2( uint32_t a )
{
    uint32_t r = 4;
    while ( r < a ) {
        r *= 2;
    }
    return r;
}

void Font::makeDlist( TTF_Font* font, uint32_t ch )
{
    SDL_Color col = { 255, 255, 255, 255 };
    SDL_Surface* tmp = nullptr;

    tmp = TTF_RenderGlyph_Blended( font, static_cast<Uint16>( ch ), col );

    int32_t minX = 0;
    int32_t maxX = 0;
    int32_t minY = 0;
    int32_t maxY = 0;
    int32_t advance = 0;
    TTF_GlyphMetrics( font, ch, &minX, &maxX, &minY, &maxY, &advance );

    uint32_t optW = pow2( tmp->w );
    uint32_t optH = pow2( m_height );
    SDL_Rect rect;
    rect.x = minX;
    rect.y = m_height - maxY;

    SDL_Surface* expanded_data = SDL_CreateRGBSurface( tmp->flags, optW, optH, 32, tmp->format->Amask, tmp->format->Gmask, tmp->format->Bmask, tmp->format->Rmask );
    SDL_BlitSurface( tmp, nullptr, expanded_data, &rect );
    SDL_FreeSurface( tmp );

    std::vector<uint16_t> pixels( optH * optW );
    const uint32_t* pix = reinterpret_cast<const uint32_t*>( expanded_data->pixels );
    const uint32_t* pixEnd = pix;
    std::advance( pixEnd, optH * optW );
    std::transform( pix, pixEnd, pixels.begin(), []( uint32_t pix ) {
        return ( pix & 0xffu ) | ( ( pix & 0xffu ) << 8u );
    } );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, m_textures[ ch ] );
    setTextureFiltering();
    gluBuild2DMipmaps( GL_TEXTURE_2D, 2, optW, optH, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixels.data() );
    SDL_FreeSurface( expanded_data );
    m_charWidth[ ch ] = advance;
    m_middlePoint = optH / 2;
    glNewList( m_listBase + ch, GL_COMPILE );
    glBindTexture( GL_TEXTURE_2D, m_textures[ ch ] );
    glPushMatrix();
    glBegin( GL_QUADS );
    glTexCoord2d( 0, 0 );
    glVertex2d( 0, optH );
    glTexCoord2d( 0, 1 );
    glVertex2d( 0, 0 );
    glTexCoord2d( 1, 1 );
    glVertex2d( optW, 0 );
    glTexCoord2d( 1, 0 );
    glVertex2d( optW, optH );
    glEnd();
    glPopMatrix();
    glTranslated( advance, 0, 0 );
    glEndList();
    m_charData[ ch ] = glm::vec3{ optW, optH, advance };
    //   glDisable(GL_TEXTURE_2D);
}

uint32_t Font::textLength( const char* text )
{
    std::string txt = text;
    uint32_t length = 0;
    for ( char i : txt ) {
        length += m_charWidth[ static_cast<size_t>( i ) ];
    }
    return length;
}

void Font::printText( double x, double y, const char* text )
{
    m_stringTxt = text;
    glListBase( m_listBase );
    glPushMatrix();
    glTranslated( x, y, 0 );
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glCallLists( m_stringTxt.length(), GL_UNSIGNED_BYTE, m_stringTxt.c_str() );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glPopMatrix();
}

uint32_t Font::height() const
{
    return m_height;
}

uint32_t Font::middlePoint() const
{
    return m_middlePoint;
}

void Font::renderText( RenderContext rctx, const glm::vec4& color, double x, double y, std::string_view text )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ x, y, 0.0 } );
    for ( char ch : text ) {
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_texture = m_textures[ ch ];
        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_color = color;
        pushConstant.m_vertices[ 0 ] = glm::vec2{ 0.0f, m_charData[ ch ].y };
        pushConstant.m_vertices[ 1 ] = glm::vec2{ 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec2{ m_charData[ ch ].x, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec2{ m_charData[ ch ].x, m_charData[ ch ].y };
        pushConstant.m_uv[ 0 ] = glm::vec2{ 0, 0 };
        pushConstant.m_uv[ 1 ] = glm::vec2{ 0, 1 };
        pushConstant.m_uv[ 2 ] = glm::vec2{ 1, 1 };
        pushConstant.m_uv[ 3 ] = glm::vec2{ 1, 0 };
        rctx.renderer->push( &pushBuffer, &pushConstant );
        rctx.model = glm::translate( rctx.model, glm::vec3{ m_charData[ ch ].z, 0.0f, 0.0f } );
    }
}
