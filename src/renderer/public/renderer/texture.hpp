#pragma once

#include <cstdint>

using Texture = uint32_t;

enum class TextureFormat : uint8_t {
    eR,
    eRGBA,
    eBGRA,
};

enum class TextureAddressMode : uint8_t {
    eClamp,
    eRepeat,
    eMirror,
};

struct TextureCreateInfo {
    uint64_t dataBeginOffset = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t mips = 0;
    TextureFormat format{};
    TextureAddressMode u{};
    TextureAddressMode v{};
};
