#pragma once

// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-header-dxt10
// https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat
// https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format


#include <cstdint>

namespace dds::dxgi {
enum class Format : uint32_t {
    UNKNOWN = 0,

    R8_UNORM = 61,

    BC1_TYPELESS = 70,
    BC1_UNORM = 71,
    BC1_UNORM_SRGB = 72,

    BC2_TYPELESS = 73,
    BC2_UNORM = 74,
    BC2_UNORM_SRGB = 75,

    BC3_TYPELESS = 76,
    BC3_UNORM = 77,
    BC3_UNORM_SRGB = 78,

    BC4_TYPELESS = 79,
    BC4_UNORM = 80,
    BC4_SNORM = 81,

    BC5_TYPELESS = 82,
    BC5_UNORM = 83,
    BC5_SNORM = 84,

    B5G6R5_UNORM = 85,
    B5G5R5A1_UNORM = 86,
    B8G8R8A8_UNORM = 87,

    B4G4R4A4_UNORM = 115,
    // TODO: more formats
};

enum class Dimension : uint32_t {
    eUnknown = 0,
    eBuffer = 1,
    eTexture1D = 2,
    eTexture2D = 3,
    eTexture3D = 4,
};

struct Header {
    Format format = {};
    Dimension dimension = Dimension::eUnknown;
    uint32_t flags = 0;
    uint32_t arraySize = 0;
    uint32_t flags2 = 0;
};
static_assert( sizeof( Header ) == 20 );

} // namespace dds::dxgi


namespace dds {

inline constexpr uint32_t MAGIC = ' SDD';
enum FourCC : uint32_t {
    ATI1 = '1ITA',
    ATI2 = '2ITA',
    BC4U = 'U4CB',
    BC5U = 'U5CB',
    DXGI = '01XD',
    DXT1 = '1TXD',
    DXT3 = '3TXD',
    DXT5 = '5TXD',
};

struct PixelFormat {
    enum Flags : uint32_t {
        fAlphaPixels = 0x1,
        fAlpha = 0x2,
        fFourCC = 0x4,
        fRGB = 0x40,
        fYUV = 0x200,
        fLuminance = 0x20000,
    };

    uint32_t size = 32;
    Flags flags = {};
    uint32_t fourCC = 0;
    uint32_t rgbBitCount = 0;
    uint32_t bitmaskR = 0;
    uint32_t bitmaskG = 0;
    uint32_t bitmaskB = 0;
    uint32_t bitmaskA = 0;
    bool operator == ( const PixelFormat& ) const noexcept = default;
};
static_assert( sizeof( PixelFormat ) == 32 );

struct Header {
    enum Flags : uint32_t {
        fCaps = 0x1,
        fHeight = 0x2,
        fWidth = 0x4,
        fPitch = 0x8,
        fPixelFormat = 0x1000,
        fMipMapCount = 0x20000,
        fLinearSize = 0x80000,
        fDepth = 0x800000,
    };
    enum Caps : uint32_t {
        fComplex = 0x8,
        fMipMap = 0x400000,
        fTexture = 0x1000,
    };

    uint32_t magic = MAGIC;
    uint32_t size = 124;
    Flags flags = {};
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitchOrLinearSize = 0;
    uint32_t depth = 0;
    uint32_t mipMapCount = 0;
    uint32_t reserved[ 11 ]{};
    PixelFormat pixelFormat{};
    Caps caps = {};
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;

};
static_assert( sizeof( Header ) == 128 );

} // namespace dds
