#ifndef SA_FONT
#define SA_FONT

#include "sa.hpp"
#include "texture.hpp"

class Font {
private:
    std::string stringtxt{};
    std::string name{};
    GLuint height = 0;
    GLuint middlepoint = 0;
    GLuint* textures = nullptr;
    GLuint* char_length = nullptr;
    GLuint list_base = 0;
    void make_dlist( TTF_Font* font, GLuint ch );
    GLuint pow2( GLint a );

public:
    Font( const char* fontname, GLuint h );
    ~Font();
    GLuint GetHeight();
    GLuint GetMiddlePoint();
    void PrintTekst( const GLdouble& x, const GLdouble& y, const char* tekst );
    GLuint GetTextLength( const char* tekst );
    GLuint GetTextLength( const std::string& tekst );
};

#endif
