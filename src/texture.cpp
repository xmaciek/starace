#include "texture.hpp"

#include "renderer.hpp"

#include <cstring>
#include <vector>

uint32_t loadDefault()
{
    uint8_t DEF[ 64 * 64 * 3 ];
    bool c = false;
    uint32_t d = 0;
    for ( uint32_t i = 0; i < 64 * 64 * 3; i++ ) {
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
    return Renderer::instance()->createTexture( 64, 64, TextureFormat::eRGB, DEF );
}

uint32_t loadTexture( std::string_view filename )
{
    uint8_t HEADER[ 12 ]{};
    const uint8_t UNCOMPRESSED[ 12 ] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FILE* TGAfile = std::fopen( filename.data(), "rb" );
    if ( !TGAfile ) {
        return loadDefault();
    }
    fread( HEADER, 12, 1, TGAfile );
    if ( std::memcmp( HEADER, UNCOMPRESSED, 12 ) != 0 ) {
        std::fclose( TGAfile );
        return loadDefault();
    }
    TGA tga{};
    std::fread( tga.header, 6, 1, TGAfile );
    tga.width = tga.header[ 1 ] * 256 + tga.header[ 0 ];
    tga.height = tga.header[ 3 ] * 256 + tga.header[ 2 ];
    tga.bpp = tga.header[ 4 ];

    if ( ( tga.width <= 0 ) || ( tga.height <= 0 ) || ( ( ( tga.bpp != 24 ) ) && ( tga.bpp != 32 ) ) ) {
        std::fclose( TGAfile );
        return loadDefault();
    }

    tga.bytesPerPixel = tga.bpp / 8;
    tga.imageSize = tga.bytesPerPixel * tga.width * tga.height;
    std::vector<uint8_t> data( tga.imageSize );
    tga.data = data.data();
    std::fread( tga.data, tga.imageSize, 1, TGAfile );
    std::fclose( TGAfile );
    uint8_t swap = 0;
    for ( uint32_t C = 0; C < tga.imageSize; C += tga.bytesPerPixel ) {
        swap = tga.data[ C + 2 ];
        tga.data[ C + 2 ] = tga.data[ C ];
        tga.data[ C ] = swap;
    }

    const TextureFormat fmt = tga.bytesPerPixel == 3 ? TextureFormat::eRGB : TextureFormat::eRGBA;

    return Renderer::instance()->createTexture( tga.width, tga.height, fmt, tga.data );
}

void destroyTexture( uint32_t tex )
{
    Renderer::instance()->deleteTexture( tex );
}
