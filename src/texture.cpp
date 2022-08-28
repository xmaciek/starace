#include "texture.hpp"

#include <renderer/renderer.hpp>
#include <extra/dds.hpp>
#include <extra/tga.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

static Texture maybeDDS( std::pmr::vector<uint8_t>& data )
{
    dds::Header header{};
    std::copy_n( data.begin(), sizeof( header ), reinterpret_cast<uint8_t*>( &header ) );
    if ( header.magic != dds::c_magic ) { return {}; }

    TextureCreateInfo tci{};
    tci.width = static_cast<uint16_t>( header.width );
    tci.height = static_cast<uint16_t>(  header.height );
    tci.mips = std::min( (uint8_t)header.mipMapCount, (uint8_t)tci.mipArray.size() );
    tci.dataBeginOffset = sizeof( header );

    const uint32_t fourcc = ( header.pixelFormat == dds::constants::bgra )
        ? 'BGRA'
        : header.pixelFormat.fourCC;
    switch ( fourcc ) {
    case 'BGRA': tci.format = TextureFormat::eBGRA; break;
    case dds::c_dxt1: tci.format = TextureFormat::eBC1_unorm; break;
    case dds::c_dxt3: tci.format = TextureFormat::eBC2_unorm; break;
    case dds::c_dxt5: tci.format = TextureFormat::eBC3_unorm; break;
    default:
        assert( !"unhandled format, here be dragons" );
        return {};
    }

    auto mipOffsetGenerator = [offset = 0u, pitchOrLinearSize = header.pitchOrLinearSize]() mutable -> TextureCreateInfo::MipArray::value_type
    {
        uintptr_t begin = offset;
        offset += pitchOrLinearSize;
        uintptr_t end = offset;
        pitchOrLinearSize >>= 2;
        return { begin, end };
    };
    std::generate_n( tci.mipArray.begin(), tci.mips, mipOffsetGenerator );
    return Renderer::instance()->createTexture( tci, std::move( data ) );
}

Texture parseTexture( std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    assert( !data.empty() );

    if ( Texture dds = maybeDDS( data ); dds ) { return dds; }

    assert( data.size() >= sizeof( tga::Header ) );
    tga::Header header{};
    auto it = data.begin();
    std::copy_n( it, sizeof( header ), reinterpret_cast<uint8_t*>( &header ) );
    std::advance( it, sizeof( header ) );
    assert( header.imageType == tga::ImageType::eTrueColor || header.imageType == tga::ImageType::eGrayscale );
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
        tci.mipArray[ 0 ] = { 0, texture.size() };
    } break;

    case 3:
    {
        ZoneScopedN( "convert texture data" );
        tci.format = TextureFormat::eBGRA;
        texture.resize( header.width * header.height * 4 );
        tci.mipArray[ 0 ] = { 0, texture.size() };
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
