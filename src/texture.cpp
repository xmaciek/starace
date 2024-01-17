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
    if ( header.magic != dds::MAGIC ) {
        assert( !"dds .magic field mismatch" );
        return {};
    }
    if ( header.size != 124 ) {
        assert( !"dds .size filed not 124" );
        return {};
    }

    using Flags = dds::Header::Flags;
    if ( ~header.flags & ( Flags::fCaps | Flags::fHeight | Flags::fWidth | Flags::fPixelFormat ) ) {
        assert( !"dds is missing necessary flags, either [ width, height, caps, pixel_format ]" );
        return {};
    }

    using Caps = dds::Header::Caps;
    if ( ~header.caps & Caps::fTexture ) {
        assert( !"dds .caps does not contain fTexture flag" );
        return {};
    }

    dataPtr += sizeof( header );

    TextureCreateInfo tci{
        .width = header.width,
        .height = header.height,
        .mip0ByteCount = header.pitchOrLinearSize,
        .dataBeginOffset = sizeof( header ),
    };

    static constexpr auto mipFlags = Caps::fComplex | Caps::fMipMap;
    if ( auto f = header.caps & mipFlags; f == mipFlags && header.flags & Flags::fMipMapCount ) {
        tci.mips = std::max( header.mipMapCount, 1u );
    }

    using enum dds::FourCC;
    switch ( header.pixelFormat.fourCC ) {
    case dds::DXGI: {
        dds::dxgi::Header dxgiHeader{};
        std::memcpy( &dxgiHeader, dataPtr, sizeof( dxgiHeader ) );
        dataPtr += sizeof( dxgiHeader );
        tci.dataBeginOffset += sizeof( dxgiHeader );
        assert( tci.dataBeginOffset == 148 );
        if ( header.caps & Caps::fComplex ) {
            tci.array = std::max( dxgiHeader.arraySize, 1u );
        }
        using enum dds::dxgi::Format;
        switch ( dxgiHeader.format ) {
        case BC1_UNORM: tci.format = TextureFormat::eBC1_unorm; break;
        case BC2_UNORM: tci.format = TextureFormat::eBC2_unorm; break;
        case BC3_UNORM: tci.format = TextureFormat::eBC3_unorm; break;
        case BC4_UNORM: tci.format = TextureFormat::eBC4_unorm; break;
        case B8G8R8A8_UNORM: tci.format = TextureFormat::eBGRA; break;
        case R8_UNORM: tci.format = TextureFormat::eR; break;
        default:
            assert( !"unhandled format, here be dragons" );
            return {};
        }
    } break;
    case DXT1: tci.format = TextureFormat::eBC1_unorm; break;
    case DXT3: tci.format = TextureFormat::eBC2_unorm; break;
    case DXT5: tci.format = TextureFormat::eBC3_unorm; break;
    case ATI1: [[fallthrough]];
    case BC4U: tci.format = TextureFormat::eBC4_unorm; break;
    case ATI2: [[fallthrough]];
    case BC5U: tci.format = TextureFormat::eBC5_unorm; break;
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
