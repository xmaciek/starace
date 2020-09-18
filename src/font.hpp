#ifndef SA_FONT
#define SA_FONT

#include "sa.hpp"
#include "texture.hpp"

class Font {
private:
    string stringtxt;
    string name;
    GLuint height, middlepoint;
    GLuint* textures;
    GLuint* char_length;
    GLuint list_base;
    void make_dlist( TTF_Font* font, GLuint ch );
    GLuint pow2( GLint a );

public:
    Font( const char* fontname, GLuint h );
    ~Font();
    GLuint GetHeight();
    GLuint GetMiddlePoint();
    void PrintTekst( const GLdouble& x, const GLdouble& y, const char* tekst );
    GLuint GetTextLength( const char* tekst );
    GLuint GetTextLength( const string& tekst );
};

#endif
