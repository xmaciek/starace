#ifndef SA_Texture_H
#define SA_Texture_H
#include "sa.hpp"

typedef struct
{
    GLubyte header[ 6 ];
    GLuint bytesPerPixel;
    GLuint imageSize;
    GLuint temp;
    GLuint type;
    GLuint height;
    GLuint width;
    GLuint bpp;
    GLubyte* data;
} TGA;

void setTextureFiltering();
GLuint LoadDefault();
GLuint LoadTexture( const char* filename );
void DrawSprite( const GLuint& spriteID, const GLdouble& spriteSize );
// extern GLint TEX_FILTER;

#endif
