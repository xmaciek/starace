#pragma once

#include "sa.hpp"
#include "texture.hpp"

class Font {
private:
    std::string m_name{};
    std::string m_stringTxt{};
    std::vector<uint32_t> m_charWidth{};
    std::vector<uint32_t> m_textures{};
    uint32_t m_height = 0;
    uint32_t m_listBase = 0;
    uint32_t m_middlePoint = 0;
    void makeDlist( TTF_Font* font, uint32_t ch );

public:
    ~Font();
    Font( const char* fontname, uint32_t h );

    uint32_t height() const;
    uint32_t middlePoint() const;
    uint32_t textLength( const char* text );
    uint32_t textLength( const std::string& text );
    void printText( double x, double y, const char* text );
};

