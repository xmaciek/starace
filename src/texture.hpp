#pragma once
#include "sa.hpp"

struct TGA {
    GLubyte header[ 6 ];
    GLuint bytesPerPixel;
    GLuint imageSize;
    GLuint temp;
    GLuint type;
    GLuint height;
    GLuint width;
    GLuint bpp;
    GLubyte* data;
};

void setTextureFiltering();
GLuint LoadDefault();
GLuint LoadTexture( const char* filename );
void DrawSprite( const GLuint& spriteID, const GLdouble& spriteSize );
