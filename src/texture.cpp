#include "texture.hpp"

#include <renderer/renderer.hpp>

#include "tga.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

Texture loadTexture( std::pmr::vector<uint8_t>&& data )
{
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
    const size_t textureSize = header.width * header.height * bytesPerPixel;

    Renderer* renderer = Renderer::instance();
    assert( renderer );
    std::pmr::vector<uint8_t> texture( textureSize, renderer->allocator() );
    std::copy_n( it, texture.size(), texture.begin() );

    TextureFormat fmt = {};
    switch ( bytesPerPixel ) {
    case 1: fmt = TextureFormat::eR; break;
    case 3: fmt = TextureFormat::eBGR; break;
    case 4: fmt = TextureFormat::eBGRA; break;
    default:
        assert( !"unhandled format" );
        break;
    }
    return renderer->createTexture( header.width, header.height, fmt, true, std::move( texture ) );
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
