#pragma once

#include <engine/math.hpp>

#include <cstdint>

struct Signal {
    math::vec3 position{};
    uint16_t team = 0xFFFF;
    uint16_t callsign = 0xFFFF;
    inline operator bool () const { return team != 0xFFFF; }
};
