#pragma once

#include <cstdint>

namespace tga {

enum ImageType : uint8_t {
    eNoImage = 0,
    eColorMap = 1,
    eTrueColor = 2,
    eGrayscale = 3,
    eRLEColorMap = 9,
    eRLETruecolor = 10,
    eRLEGrayscale = 11,
};

struct Header {
    uint8_t idLength = 0;
    uint8_t mapType = 0;
    ImageType imageType = ImageType::eNoImage;
    uint8_t colorMap[ 5 ]{};
    uint16_t xOrigin = 0;
    uint16_t yOrigin = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t bitsPerPixel = 0;
    uint8_t imageDescriptor = 0;
};
static_assert( sizeof( tga::Header ) == 18 );

}
