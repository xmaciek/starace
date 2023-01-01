#pragma once

#include <array>
#include <cstdint>
#include <tuple>

using Texture = uint32_t;

enum class TextureFormat : uint8_t {
    eUnknown,
    eR,
    eRGBA,
    eBGRA,
    eBC1_unorm,
    eBC2_unorm,
    eBC3_unorm,
};

enum class TextureAddressMode : uint8_t {
    eClamp,
    eRepeat,
    eMirror,
};

struct TextureCreateInfo {
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t mip0ByteCount = 0;
    uint32_t dataBeginOffset = 0;
    uint8_t mips = 1;
    TextureFormat format{};
    TextureAddressMode u{};
    TextureAddressMode v{};
};
