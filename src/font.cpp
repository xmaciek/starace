#include "font.hpp"

#include <algorithm>

Font::Font( const char* fontname, GLuint h )
: m_name( fontname )
, m_charWidth( 128 )
, m_textures( 128 )
, m_height( h )
{
    m_listBase = glGenLists( 128 );
    glGenTextures( 128, m_textures.data() );
    TTF_Font* font = TTF_OpenFont( fontname, h );

    for ( GLubyte i = 0; i < 128; i++ ) {
        makeDlist( font, i );
    }
    TTF_CloseFont( font );
}

Font::~Font()
{
    glDeleteLists( m_listBase, 128 );
    glDeleteTextures( 128, m_textures.data() );
}

static GLuint pow2( GLuint a )
{
    GLuint r = 4;
    while ( r < a ) {
        r *= 2;
    }
    return r;
}

void Font::makeDlist( TTF_Font* font, GLuint ch )
{
    SDL_Color col = { 255, 255, 255, 255 };
    SDL_Surface* tmp = nullptr;

    tmp = TTF_RenderGlyph_Blended( font, static_cast<Uint16>( ch ), col );

    GLint minX = 0;
    GLint maxX = 0;
    GLint minY = 0;
    GLint maxY = 0;
    GLint advance = 0;
    TTF_GlyphMetrics( font, ch, &minX, &maxX, &minY, &maxY, &advance );

    GLuint optW = pow2( tmp->w );
    GLuint optH = pow2( m_height );
    SDL_Rect rect;
    rect.x = minX;
    rect.y = m_height - maxY;

    SDL_Surface* expanded_data = SDL_CreateRGBSurface( tmp->flags, optW, optH, 32, tmp->format->Amask, tmp->format->Gmask, tmp->format->Bmask, tmp->format->Rmask );
    SDL_BlitSurface( tmp, nullptr, expanded_data, &rect );
    SDL_FreeSurface( tmp );

    std::vector<GLushort> pixels( optH * optW );
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
    //   glDisable(GL_TEXTURE_2D);
}

GLuint Font::textLength( const char* text )
{
    std::string txt = text;
    GLuint length = 0;
    for ( char i : txt ) {
        length += m_charWidth[ static_cast<size_t>( i ) ];
    }
    return length;
}

void Font::printText( GLdouble x, GLdouble y, const char* text )
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

GLuint Font::height() const
{
    return m_height;
}

GLuint Font::middlePoint() const
{
    return m_middlePoint;
}
