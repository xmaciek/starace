#pragma once

#include "SA.h"
#include "Texture.h"

#include <map>

struct Glyph {
    Glyph();
    uint32_t m_textureID;
    uint32_t m_bufferID;
    uint32_t m_textureCoordID;
    int32_t m_width;
};

class Font {
private:
    std::string m_name;
    uint32_t m_height, m_middlePoint;

    // <character value, Glyph>
    typedef std::map<uint32_t, Glyph> GlyphMap;
    GlyphMap m_glyphMap;
    void createGlyphFor( uint32_t characterValue, TTF_Font* font );

public:
    Font( const std::string& fontName, uint32_t h );
    ~Font();
    uint32_t GetHeight() const ;
    uint32_t GetMiddlePoint() const ;
    void PrintTekst( double x, double y, const std::string& text ) const;
    uint32_t GetTextLength( const std::string& text ) const;
};
