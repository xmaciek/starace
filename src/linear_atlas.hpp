#pragma once

#include <engine/math.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

using Sprite = std::array<uint16_t, 4>; // x, y, w, h

class LinearAtlas {
public:

private:
    uint16_t m_width = 0;
    uint16_t m_height = 0;
    std::array<Sprite, 16> m_sprites{};

public:
    LinearAtlas() noexcept = default;
    LinearAtlas( std::span<const Sprite>, uint16_t width, uint16_t height ) noexcept;

    math::vec2 extent() const;
    math::vec4 sliceUV( uint32_t ) const; // returns uv in xywh style
    Sprite sprite( uint32_t ) const;

    std::array<math::vec2, 4> composeUV4( uint32_t ) const;
    std::array<math::vec2, 6> composeUV6( uint32_t ) const;

};
