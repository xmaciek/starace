#include "texture.hpp"

#include <renderer/renderer.hpp>
#include <extra/dds.hpp>
#include <extra/tga.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

Texture parseTexture( std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    const uint8_t* dataPtr = data.data();
    dds::Header header{};
    std::memcpy( &header, dataPtr, sizeof( header ) );
    if ( header.magic != dds::c_magic ) { return {}; }
    dataPtr += sizeof( header );

    TextureCreateInfo tci{
        .dataBeginOffset = sizeof( header ),
        .width = static_cast<uint16_t>( header.width ),
        .height = static_cast<uint16_t>(  header.height ),
    };
    tci.mips = std::clamp( (uint8_t)header.mipMapCount, (uint8_t)1, (uint8_t)tci.mipArray.size() );


    switch ( header.pixelFormat.fourCC ) {
    case dds::c_dxgi: {
        dds::dxgi::Header dxgiHeader{};
        std::memcpy( &dxgiHeader, dataPtr, sizeof( dxgiHeader ) );
        dataPtr += sizeof( dxgiHeader );
        tci.dataBeginOffset += sizeof( dxgiHeader );
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

    auto mipOffsetGenerator = [offset = 0u, byteCount = header.pitchOrLinearSize]() mutable -> TextureCreateInfo::MipArray::value_type
    {
        uintptr_t begin = offset;
        offset += byteCount;
        uintptr_t end = offset;
        byteCount >>= 2;
        return { begin, end };
    };
    std::generate_n( tci.mipArray.begin(), tci.mips, mipOffsetGenerator );
    return Renderer::instance()->createTexture( tci, std::move( data ) );
}


void destroyTexture( Texture tex )
{
    Renderer::instance()->deleteTexture( tex );
}
