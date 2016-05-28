#include "Font.h"

Glyph::Glyph() :
    m_textureID( 0 ),
    m_bufferID( 0 ),
    m_textureCoordID( 0 ),
    m_width( 0 )
{}

Font::Font( const std::string& fontName, uint32_t h ) :
    m_name( fontName ),
    m_height( h ),
    m_middlePoint( h / 2 )
{
    std::cout << "+-- Creating Font " << fontName << ":" << h << std::endl;

    TTF_Font *font = TTF_OpenFont( fontName.c_str(), h );
    for ( uint32_t i = 0; i < 128; i++ ) {
        createGlyphFor( i, font );
    }
    TTF_CloseFont( font );
}

Font::~Font() {}

static uint32_t pow2( uint32_t a ) {
    uint32_t r = 4;
    while ( r < a ) { r *= 2; }
    return r;
}

void Font::createGlyphFor( uint32_t characterValue, TTF_Font* font ) {
    Glyph glyph;
    glGenTextures( 1, &glyph.m_textureID );
    glyph.m_textureCoordID = SHADER::getQuadTextureCoord( 0, 1, 1, 0 );
    {
        SDL_Color color = { 255, 255, 255 };
        SDL_Surface* tmp = TTF_RenderGlyph_Blended( font, characterValue, color );

        int32_t minX, maxX, minY, maxY;
        TTF_GlyphMetrics( font, characterValue, &minX, &maxX, &minY, &maxY, &glyph.m_width );

        uint32_t optW = pow2( tmp->w );
        uint32_t optH = pow2( m_height );
        glyph.m_bufferID = SHADER::getQuad( 0, 0, optW, optH );

        SDL_Rect rect;
        rect.x = minX;
        rect.y = m_height - maxY;

        SDL_Surface* expanded_data = SDL_CreateRGBSurface( tmp->flags, optW, optH, 32,
            tmp->format->Amask, tmp->format->Gmask, tmp->format->Bmask, tmp->format->Rmask );
        SDL_BlitSurface( tmp, NULL, expanded_data, &rect );
        SDL_FreeSurface( tmp );

        //for some reason, SDL creates yellow glyph textures, I want them white
        uint8_t* pixPtr = (uint8_t*)expanded_data->pixels;
        for ( uint32_t i = 0; i < optH * optW * 4; i += 4 ) {
            pixPtr[i] = pixPtr[i];
            pixPtr[i + 1] = pixPtr[i];
            pixPtr[i + 2] = pixPtr[i];
        }

        glBindTexture( GL_TEXTURE_2D, glyph.m_textureID );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, optW, optH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixPtr );
        glGenerateMipmap( GL_TEXTURE_2D );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
        SDL_FreeSurface( expanded_data );

        m_middlePoint = optH/2;
    }
    m_glyphMap.insert( std::make_pair( characterValue, glyph ) );
}


uint32_t Font::GetTextLength( const std::string& text ) const {
    uint32_t length = 0;
    std::string::const_iterator it = text.begin();
    while ( it != text.end() ) {
        GlyphMap::const_iterator git = m_glyphMap.find( *it );
        if ( git != m_glyphMap.end() ) {
            length += git->second.m_width;
        }
        it++;
    }
    return length;
}

void Font::PrintTekst( double x, double y, const std::string& text ) const {
    SHADER::translate( x, y, 0 );
    std::string::const_iterator it = text.begin();
    while ( it != text.end() ) {
        GlyphMap::const_iterator git = m_glyphMap.find( *it );
        if ( git != m_glyphMap.end() ) {
            glBindTexture( GL_TEXTURE_2D, git->second.m_textureID );
            SHADER::setTextureCoord( git->second.m_textureCoordID );
            SHADER::draw( GL_TRIANGLES, git->second.m_bufferID, 6 );
            SHADER::translate( git->second.m_width, 0, 0 );
        }
        it++;
    }
}


uint32_t Font::GetHeight() const { return m_height; }
uint32_t Font::GetMiddlePoint() const { return m_middlePoint; }
