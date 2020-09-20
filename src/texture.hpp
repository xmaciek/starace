#pragma once
#include "sa.hpp"

struct TGA {
    GLubyte header[ 6 ]{};
    GLuint bytesPerPixel = 0;
    GLuint imageSize = 0;
    GLuint temp = 0;
    GLuint type = 0;
    GLuint height = 0;
    GLuint width = 0;
    GLuint bpp = 0;
    GLubyte* data = nullptr;
};

void setTextureFiltering();
GLuint LoadDefault();
GLuint LoadTexture( const char* filename );
void DrawSprite( const GLuint& spriteID, const GLdouble& spriteSize );
