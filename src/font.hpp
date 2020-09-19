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
    std::vector<GLuint> textures{};
    std::vector<GLuint> char_length{};
    GLuint list_base = 0;
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
