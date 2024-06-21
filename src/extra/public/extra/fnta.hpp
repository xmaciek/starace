#pragma once

#include <cstdint>

namespace fnta {

struct Header {
    static constexpr inline uint32_t MAGIC = 'ATNF';
    static constexpr inline uint32_t VERSION = 2;
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t count = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t lineHeight = 0;
};

struct Glyph {
    uint16_t position[ 2 ]{};
    uint16_t size[ 2 ]{};
    int16_t advance[ 2 ]{};
    int16_t padding[ 2 ]{};
    inline operator bool () const { return size[ 0 ] || size[ 1 ]; }
};

}
