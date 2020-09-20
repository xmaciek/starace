#include "texture.hpp"

GLuint loadDefault()
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
    setTextureFiltering();
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, 64, 64, GL_RGB, GL_UNSIGNED_BYTE, DEF );
    return textureID;
}

void setTextureFiltering()
{
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
}

GLuint loadTexture( const char* filename )
{
    GLubyte HEADER[ 12 ]{};
    const GLubyte UNCOMPRESSED[ 12 ] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FILE* TGAfile = fopen( filename, "rb" );
    if ( !TGAfile ) {
        return loadDefault();
    }
    fread( HEADER, 12, 1, TGAfile );
    if ( memcmp( HEADER, UNCOMPRESSED, 12 ) != 0 ) {
        fclose( TGAfile );
        return loadDefault();
    }
    TGA tga{};
    fread( tga.header, 6, 1, TGAfile );
    tga.width = tga.header[ 1 ] * 256 + tga.header[ 0 ];
    tga.height = tga.header[ 3 ] * 256 + tga.header[ 2 ];
    tga.bpp = tga.header[ 4 ];

    if ( ( tga.width <= 0 ) || ( tga.height <= 0 ) || ( ( ( tga.bpp != 24 ) ) && ( tga.bpp != 32 ) ) ) {
        fclose( TGAfile );
        return loadDefault();
    }

    if ( tga.bpp == 24 ) {
        tga.type = GL_RGB;
    }
    else {
        tga.type = GL_RGBA;
    }
    tga.bytesPerPixel = tga.bpp / 8;
    tga.imageSize = tga.bytesPerPixel * tga.width * tga.height;
    tga.data = new GLubyte[ tga.imageSize ];
    fread( tga.data, tga.imageSize, 1, TGAfile );
    fclose( TGAfile );
    GLubyte swap = 0;
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
    GLuint textureID = 0;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    setTextureFiltering();
    gluBuild2DMipmaps( GL_TEXTURE_2D, tga.bytesPerPixel, tga.width, tga.height, tga.type, GL_UNSIGNED_BYTE, tga.data );
    delete[] tga.data;

    glBindTexture( GL_TEXTURE_2D, 0 );
    return textureID;
}

void drawSprite( GLuint spriteID, GLdouble spriteSize )
{
    GLfloat matrix[ 16 ]{};
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
