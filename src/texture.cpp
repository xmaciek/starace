#include "texture.hpp"

#include <renderer/renderer.hpp>

#include "tga.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

Texture loadDefault()
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
    return Renderer::instance()->createTexture( 64, 64, Texture::Format::eRGB, false, DEF );
}

Texture loadTexture( std::pmr::vector<uint8_t>&& data )
{
    assert( data.size() >= sizeof( tga::Header ) );
    tga::Header header{};
    auto it = data.begin();
    std::copy_n( it, sizeof( header ), reinterpret_cast<uint8_t*>( &header ) );
    std::advance( it, sizeof( header ) );
    assert( header.imageType == tga::ImageType::eTrueColor );
    assert( header.width > 0 );
    assert( header.height > 0 );
    const size_t bytesPerPixel = header.bitsPerPixel / 8;
    assert( bytesPerPixel == 3 || bytesPerPixel == 4 );
    const size_t textureSize = header.width * header.height * bytesPerPixel;

    std::vector<uint8_t> texture( textureSize );
    std::copy_n( it, texture.size(), texture.begin() );

    if ( bytesPerPixel == 3 ) {
        std::vector<uint8_t> tmp( header.width * header.height * 4 );
        struct BGR { uint8_t b,g,r; };
        struct RGBA { uint8_t r,g,b,a; };
        BGR* begin = reinterpret_cast<BGR*>( texture.data() );
        BGR* end = begin + header.width * header.height;
        RGBA* dataOut = reinterpret_cast<RGBA*>( tmp.data() );
        std::transform( begin, end, dataOut, []( BGR bgr ) { return RGBA{ bgr.r, bgr.g, bgr.b, 255u }; } );
        texture = std::move( tmp );
    } else {
        for ( uint8_t* ptr = texture.data(); ptr < &texture.back(); ptr += 4 ) {
            std::swap( ptr[ 0 ], ptr[ 2 ] );
        }
    }

    return Renderer::instance()->createTexture( header.width, header.height, Texture::Format::eRGBA, true, texture.data() );
}

Texture loadTexture( std::string_view filename )
{
    std::ifstream ifs( std::string( filename ), std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );

    const size_t size = ifs.tellg();
    ifs.seekg( 0 );
    std::pmr::vector<uint8_t> data( size );
    ifs.read( reinterpret_cast<char*>( data.data() ), (uint32_t)data.size() );
    ifs.close();
    return loadTexture( std::move( data ) );
}

void destroyTexture( Texture tex )
{
    Renderer::instance()->deleteTexture( tex );
}
