#pragma once

#include <cstdint>

namespace fnta {

struct Input {
    enum Enum : char32_t {
        base = 0xE100,
        A = base, B, X, Y,
        Start, Select, Touch,
        Up, Down, Left, Right,
        LB, LT, LX, LY, L,
        RB, RT, RX, RY, R,
    };
};

struct Header {
    static constexpr inline uint32_t MAGIC = 'ATNF';
    static constexpr inline uint32_t VERSION = 3;
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t count = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t lineHeight = 0;
    uint32_t textureHash = 0;
};

struct Glyph {
    uint16_t position[ 2 ]{};
    uint16_t size[ 2 ]{};
    int16_t advance[ 2 ]{};
    int16_t padding[ 2 ]{};
    inline operator bool () const { return size[ 0 ] || size[ 1 ]; }
};

}
