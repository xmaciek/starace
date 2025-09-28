#pragma once

#include <array>
#include <cstdint>
#include <tuple>

using Texture = uint32_t;

enum class TextureFormat : uint8_t {
    eUnknown,
    eR,
    eBGRA,
    eBC1_unorm,
    eBC2_unorm,
    eBC3_unorm,
    eBC4_unorm,
    eBC5_unorm,
    eB4G4R4A4_unorm,
};

enum class TextureAddressMode : uint8_t {
    eClamp,
    eRepeat,
    eMirror,
};

struct TextureCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mip0ByteCount = 0;
    uint32_t mips = 1;
    uint32_t array = 1;
    TextureFormat format{};
    TextureAddressMode u{};
    TextureAddressMode v{};
};
