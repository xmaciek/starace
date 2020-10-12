#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <string_view>

struct TGA {
    uint8_t header[ 6 ]{};
    uint32_t bytesPerPixel = 0;
    uint32_t imageSize = 0;
    uint32_t temp = 0;
    uint32_t type = 0;
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t bpp = 0;
    uint8_t* data = nullptr;
};

Texture loadDefault();
Texture loadTexture( std::string_view );
void destroyTexture( Texture );
