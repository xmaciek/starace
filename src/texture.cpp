#include "texture.hpp"

GLuint LoadDefault()
{
    GLubyte DEF[ 64 * 64 * 3 ];
    bool c = false;
    GLuint d = 0;
    for ( GLuint i = 0; i < 64 * 64 * 3; i++ ) {
        if ( i % ( 64 * 3 * 8 ) == 0 ) {
            c = !c;
        }
        if ( ( d < 8 && c ) || ( d >= 8 && !c ) ) {
            DEF[ i ] = 255;
            DEF[ i + 1 ] = 0;
            DEF[ i + 2 ] = 192;
        }
        else {
            DEF[ i ] = DEF[ i + 1 ] = DEF[ i + 2 ] = 0;
        }
        i += 2;
        d++;
        if ( d >= 16 ) {
            d = 0;
        }
    }
    GLuint textureID = 0;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    setTextureFiltering( -1 );
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, 64, 64, GL_RGB, GL_UNSIGNED_BYTE, DEF );

    //   glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, DEF);
    return textureID;
}

void setAllTexturesFiltering( GLint type )
{
    for ( GLuint i = 0; i < 1000; i++ ) {
        if ( glIsTexture( i ) ) {
            //       cout<<"Tex: "<<i<<"\n";
            glBindTexture( GL_TEXTURE_2D, i );
            setTextureFiltering( type );
        }
    }
}

void setTextureFiltering( GLint type )
{
    static GLint TEX_FILTER = FILTERING_TRILINEAR; /*trilinear by default*/
    if ( ( type >= FILTERING_TRILINEAR ) && ( type <= FILTERING_ANISOTROPIC_X16 ) ) {
        TEX_FILTER = type;
    }

    //   cout<<"Filtering: "<<TEX_FILTER<<"\n";

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
    //   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,0.0f);
    switch ( TEX_FILTER ) {
    case FILTERING_NONE:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
        break;
    case FILTERING_LINEAR:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
        break;
    case FILTERING_BILINEAR:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
        break;
    case FILTERING_TRILINEAR:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f );
        break;
    case FILTERING_ANISOTROPIC_X2:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f );
        break;
    case FILTERING_ANISOTROPIC_X4:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f );
        break;
    case FILTERING_ANISOTROPIC_X8:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f );
        break;
    case FILTERING_ANISOTROPIC_X16:
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
        break;
    default:
        break;
    }
}

GLuint LoadTexture( const char* filename )
{
    std::cout << "loading texture " << filename << " ... ";
    GLubyte HEADER[ 12 ]; // = new GLubyte[12];
    GLubyte UNCOMPRESSED[ 12 ] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FILE* TGAfile = fopen( filename, "rb" );
    if ( TGAfile == NULL ) {
        std::cout << "file not found, loading default.\n";
        return LoadDefault();
    }
    fread( HEADER, 12, 1, TGAfile );
    if ( memcmp( HEADER, UNCOMPRESSED, 12 ) != 0 ) {
        fclose( TGAfile );
        //     delete[] HEADER;
        std::cout << "File " << filename << " is not uncompressed RLE! Using default.\n";
        return LoadDefault();
    }
    TGA tga;
    fread( tga.header, 6, 1, TGAfile );
    tga.width = tga.header[ 1 ] * 256 + tga.header[ 0 ];
    tga.height = tga.header[ 3 ] * 256 + tga.header[ 2 ];
    tga.bpp = tga.header[ 4 ];

    if ( ( tga.width <= 0 ) || ( tga.height <= 0 ) || ( ( ( tga.bpp != 24 ) ) && ( tga.bpp != 32 ) ) ) {
        fclose( TGAfile );
        std::cout << "Texture file has invalid dimension or has invalid bit depth. Using default.\n";
        return LoadDefault();
    }

    if ( tga.bpp == 24 ) {
        tga.type = GL_RGB;
    }
    else {
        tga.type = GL_RGBA;
    }
    tga.bytesPerPixel = tga.bpp / 8;
    tga.imageSize = tga.bytesPerPixel * tga.width * tga.height;
    std::cout << " " << tga.width << "x" << tga.height << ":" << tga.bpp << "\n";
    tga.data = new GLubyte[ tga.imageSize ];
    fread( tga.data, tga.imageSize, 1, TGAfile );
    fclose( TGAfile );
    GLubyte swap;
    //   for (GLuint x=0; x<tga.imageSize; x++) { printf("%X ", tga.data[x]); }
    for ( GLuint C = 0; C < tga.imageSize; C += tga.bytesPerPixel ) {
        swap = tga.data[ C + 2 ];
        tga.data[ C + 2 ] = tga.data[ C ];
        tga.data[ C ] = swap;
    }

    if ( tga.bytesPerPixel == 3 ) {
        tga.type = GL_RGB;
    }
    else {
        tga.type = GL_RGBA;
    }
    GLuint textureID;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    setTextureFiltering( -1 );
    gluBuild2DMipmaps( GL_TEXTURE_2D, tga.bytesPerPixel, tga.width, tga.height, tga.type, GL_UNSIGNED_BYTE, tga.data );
    delete[] tga.data;

    glBindTexture( GL_TEXTURE_2D, 0 );
    return textureID;
}

void DrawSprite( const GLuint& spriteID, const GLdouble& spriteSize )
{
    static GLfloat matrix[ 16 ];
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glGetFloatv( GL_PROJECTION, matrix );
    glLoadIdentity();
    glOrtho( -1, 1, -1, 1, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glBindTexture( GL_TEXTURE_2D, spriteID );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 1 );
    glVertex2d( -spriteSize, -spriteSize );
    glTexCoord2f( 1, 1 );
    glVertex2d( spriteSize, -spriteSize );
    glTexCoord2f( 1, 0 );
    glVertex2d( spriteSize, spriteSize );
    glTexCoord2f( 0, 0 );
    glVertex2d( -spriteSize, spriteSize );
    glEnd();
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( matrix );
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}
