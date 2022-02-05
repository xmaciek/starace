#include "texture.hpp"

#include <renderer/renderer.hpp>

#include "tga.hpp"

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

Texture loadTexture( std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    assert( !data.empty() );
    assert( data.size() >= sizeof( tga::Header ) );
    tga::Header header{};
    auto it = data.begin();
    std::copy_n( it, sizeof( header ), reinterpret_cast<uint8_t*>( &header ) );
    std::advance( it, sizeof( header ) );
    assert( header.imageType == tga::ImageType::eTrueColor );
    assert( header.width > 0 );
    assert( header.height > 0 );
    const size_t bytesPerPixel = header.bitsPerPixel / 8;

    Renderer* renderer = Renderer::instance();
    assert( renderer );
    std::pmr::vector<uint8_t> texture( renderer->allocator() );


    TextureCreateInfo tci{
        .width = header.width,
        .height = header.height,
        .mips = 1,
        .u = TextureAddressMode::eClamp,
        .v = TextureAddressMode::eClamp,
    };

    switch ( bytesPerPixel ) {
    case 1:
    case 4:
    {
        tci.dataBeginOffset = sizeof( tga::Header );
        tci.format = bytesPerPixel == 1 ? TextureFormat::eR : TextureFormat::eBGRA;
        texture = std::move( data );
    } break;

    case 3:
    {
        ZoneScopedN( "convert texture data" );
        tci.format = TextureFormat::eBGRA;
        texture.resize( header.width * header.height * 4 );
        using BGR = uint8_t[3];
        const BGR* bgrBegin = reinterpret_cast<const BGR*>( &*it );
        const BGR* bgrEnd = bgrBegin + header.width * header.height;
        uint32_t* bgra = reinterpret_cast<uint32_t*>( texture.data() );
        std::transform( bgrBegin, bgrEnd, bgra, []( const BGR& rgb )
        {
            return ( (uint32_t)rgb[0] << 0 )
                | ( (uint32_t)rgb[1] << 8 )
                | ( (uint32_t)rgb[2] << 16 )
                | 0xff000000;
        } );
    } break;

    default:
        assert( !"unhandled format" );
        break;
    }
    return renderer->createTexture( tci, std::move( texture ) );
}

void destroyTexture( Texture tex )
{
    Renderer::instance()->deleteTexture( tex );
}
