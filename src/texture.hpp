#pragma once
#include "sa.hpp"

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

uint32_t loadDefault();
uint32_t loadTexture( const char* filename );
void drawSprite( uint32_t spriteID, double spriteSize );
void setTextureFiltering();
