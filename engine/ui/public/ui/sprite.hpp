#pragma once

#include <math.hpp>
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
    inline math::vec4 uvwh() const { return xyuv; }
    inline math::vec4 geometry() const { return math::vec4{ x, y, w, h }; }
    inline math::vec2 extent() const { return math::vec2{ w, h }; }
    inline operator Texture () const { return texture; }
};

}
