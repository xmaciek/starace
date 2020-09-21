#pragma once

#include "sa.hpp"
#include "texture.hpp"

class Font {
private:
    std::string m_name{};
    std::string m_stringTxt{};
    std::vector<GLuint> m_charWidth{};
    std::vector<GLuint> m_textures{};
    GLuint m_height = 0;
    GLuint m_listBase = 0;
    GLuint m_middlePoint = 0;
    void makeDlist( TTF_Font* font, GLuint ch );

public:
    ~Font();
    Font( const char* fontname, GLuint h );

    GLuint height() const;
    GLuint middlePoint() const;
    GLuint textLength( const char* text );
    GLuint textLength( const std::string& text );
    void printText( GLdouble x, GLdouble y, const char* text );
};

