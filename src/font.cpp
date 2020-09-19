#include "font.hpp"

Font::Font( const char* fontname, GLuint h )
{
    std::cout << "+-- Creating Font " << fontname << ":" << h << "\n";
    name = fontname;
    textures = new GLuint[ 128 ];
    char_length = new GLuint[ 128 ];
    height = h;
    //   middlepoint = h/2;

    list_base = glGenLists( 128 );
    glGenTextures( 128, textures );
    TTF_Font* font = TTF_OpenFont( fontname, h );

    for ( GLubyte i = 0; i < 128; i++ ) {
        make_dlist( font, i );
    }
    TTF_CloseFont( font );
}

Font::~Font()
{
    std::cout << "+-- Deleting Font " << name << ":" << height << "\n";
    glDeleteLists( list_base, 128 );
    glDeleteTextures( 128, textures );
    delete[] textures;
}

GLuint Font::pow2( GLint a )
{
    GLuint r = 4;
    while ( r < (GLuint)a ) {
        r *= 2;
    }
    return r;
}

void Font::make_dlist( TTF_Font* font, GLuint ch )
{
    SDL_Color col = { 255, 255, 255, 255 };
    SDL_Surface* tmp = NULL;

    tmp = TTF_RenderGlyph_Blended( font, (Uint16)ch, col );

    GLint minX, maxX, minY, maxY, advance;
    TTF_GlyphMetrics( font, ch, &minX, &maxX, &minY, &maxY, &advance );

    GLuint optW = pow2( tmp->w ), optH = pow2( height );
    SDL_Rect rect;
    rect.x = minX;
    rect.y = height - maxY;

    SDL_Surface* expanded_data = SDL_CreateRGBSurface( tmp->flags, optW, optH, 32, tmp->format->Amask, tmp->format->Gmask, tmp->format->Bmask, tmp->format->Rmask );
    SDL_BlitSurface( tmp, NULL, expanded_data, &rect );
    SDL_FreeSurface( tmp );

    GLuint index = 0;
    GLubyte pixels[ optH * optW * 2 ];
    GLubyte* pix = (GLubyte*)expanded_data->pixels;
    for ( GLuint i = 0; i < ( optH ) * (optW)*4; i += 4 ) {
        pixels[ index ] = pix[ i ];
        pixels[ index + 1 ] = pix[ i ];
        index += 2;
    }

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, textures[ ch ] );
    setTextureFiltering();
    gluBuild2DMipmaps( GL_TEXTURE_2D, 2, optW, optH, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixels );
    SDL_FreeSurface( expanded_data );
    char_length[ ch ] = advance;
    middlepoint = optH / 2;
    glNewList( list_base + ch, GL_COMPILE );
    glBindTexture( GL_TEXTURE_2D, textures[ ch ] );
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

GLuint Font::GetTextLength( const char* tekst )
{
    std::string txt = tekst;
    GLuint length = 0;
    for ( GLuint i = 0; i < txt.size(); i++ ) {
        length += char_length[ (int)txt[ i ] ];
    }
    return length;
}

void Font::PrintTekst( const GLdouble& x, const GLdouble& y, const char* tekst )
{
    stringtxt = tekst;
    glListBase( list_base );
    glPushMatrix();
    glTranslated( x, y, 0 );
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    //     glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glCallLists( stringtxt.length(), GL_UNSIGNED_BYTE, stringtxt.c_str() );
    //     glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glPopMatrix();
}

GLuint Font::GetHeight()
{
    return height;
}
GLuint Font::GetMiddlePoint()
{
    return middlepoint;
}
