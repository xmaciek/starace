#pragma once

#include <cstdint>

using Texture = uint32_t;

enum class TextureFormat {
    eR,
    eRGB,
    eRGBA,
    eBGRA,
    eBGR,
};
