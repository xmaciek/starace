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

GLuint loadDefault();
GLuint loadTexture( const char* filename );
void drawSprite( GLuint spriteID, GLdouble spriteSize );
void setTextureFiltering();
