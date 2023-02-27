#include "texture.hpp"

#include <renderer/renderer.hpp>
#include <extra/dds.hpp>
#include <extra/tga.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

template <typename T>
static Texture parseTexture( const T& data )
{
    ZoneScoped;
    const uint8_t* dataPtr = data.data();
    dds::Header header{};
    std::memcpy( &header, dataPtr, sizeof( header ) );
    if ( header.magic != dds::c_magic ) {
        assert( !"dds .magic field mismatch" );
        return {};
    }
    if ( header.size != 124 ) {
        assert( !"dds .size filed not 124" );
        return {};
    }
    dataPtr += sizeof( header );

    TextureCreateInfo tci{
        .width = static_cast<uint16_t>( header.width ),
        .height = static_cast<uint16_t>( header.height ),
        .mip0ByteCount = header.pitchOrLinearSize,
        .dataBeginOffset = sizeof( header ),
        .mips = static_cast<uint8_t>( std::max( header.mipMapCount, 1u ) ),
    };


    switch ( header.pixelFormat.fourCC ) {
    case dds::c_dxgi: {
        dds::dxgi::Header dxgiHeader{};
        std::memcpy( &dxgiHeader, dataPtr, sizeof( dxgiHeader ) );
        dataPtr += sizeof( dxgiHeader );
        tci.dataBeginOffset += sizeof( dxgiHeader );
        assert( tci.dataBeginOffset == 148 );
        using enum dds::dxgi::Format;
        switch ( dxgiHeader.format ) {
        case BC1_UNORM: tci.format = TextureFormat::eBC1_unorm; break;
        case BC2_UNORM: tci.format = TextureFormat::eBC2_unorm; break;
        case BC3_UNORM: tci.format = TextureFormat::eBC3_unorm; break;
        case B8G8R8A8_UNORM: tci.format = TextureFormat::eBGRA; break;
        case R8_UNORM: tci.format = TextureFormat::eR; break;
        default:
            assert( !"unhandled format, here be dragons" );
            return {};
        }
    } break;
    case dds::c_dxt1: tci.format = TextureFormat::eBC1_unorm; break;
    case dds::c_dxt3: tci.format = TextureFormat::eBC2_unorm; break;
    case dds::c_dxt5: tci.format = TextureFormat::eBC3_unorm; break;
    default:
        assert( !"unhandled format, here be dragons" );
        return {};
    }

    return Renderer::instance()->createTexture( tci, data );
}

Texture parseTexture( std::span<const uint8_t> span )
{
    return parseTexture<std::span<const uint8_t>>( span );
}

Texture parseTexture( std::pmr::vector<uint8_t>&& vec )
{
    return parseTexture<std::pmr::vector<uint8_t>>( vec );
}


void destroyTexture( Texture tex )
{
    Renderer::instance()->deleteTexture( tex );
}
