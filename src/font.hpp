#ifndef SA_FONT
#define SA_FONT

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
    void make_dlist( TTF_Font* font, GLuint ch );

public:
    Font( const char* fontname, GLuint h );
    ~Font();
    GLuint GetHeight() const;
    GLuint GetMiddlePoint() const;
    void PrintTekst( const GLdouble& x, const GLdouble& y, const char* tekst );
    GLuint GetTextLength( const char* tekst );
    GLuint GetTextLength( const std::string& tekst );
};

#endif
