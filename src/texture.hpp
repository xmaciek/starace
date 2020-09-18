#ifndef SA_Texture_H
#define SA_Texture_H
#include "SA.h"
using namespace std;
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
#define FILTERING_NONE 0
#define FILTERING_LINEAR 1
#define FILTERING_BILINEAR 2
#define FILTERING_TRILINEAR 3
#define FILTERING_ANISOTROPIC_X2 4
#define FILTERING_ANISOTROPIC_X4 5
#define FILTERING_ANISOTROPIC_X8 6
#define FILTERING_ANISOTROPIC_X16 7

void setAllTexturesFiltering( GLint type );
void setTextureFiltering( GLint type );
GLuint LoadDefault();
GLuint LoadTexture( const char* filename );
void DrawSprite( const GLuint& spriteID, const GLdouble& spriteSize );
// extern GLint TEX_FILTER;

#endif