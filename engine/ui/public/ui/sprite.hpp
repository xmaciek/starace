#pragma once

#include <engine/math.hpp>
#include <renderer/texture.hpp>

#include <cstdint>

namespace ui {

struct Sprite {
    math::vec4 xyuv{};
    Texture texture{};
    uint16_t x{};
    uint16_t y{};
    uint16_t w{};
    uint16_t h{};
    inline operator math::vec4 () const { return xyuv; }
    inline operator Texture () const { return texture; }
};

}
