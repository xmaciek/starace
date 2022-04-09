#pragma once

#include <engine/math.hpp>

#include <array>
#include <cstdint>

class LinearAtlas;

namespace spritegen {

struct NineSliceComposer {
    const LinearAtlas* m_atlas = nullptr;
    std::array<uint32_t, 9> m_spriteIds{};
    uint32_t m_currentVert = 0;
    math::vec4 m_xyBegin{};
    math::vec4 m_xyOffset{};
    math::vec2 m_midStretch{};

    static constexpr uint32_t count() noexcept { return 54u; }
    math::vec4 operator () () noexcept;
};

struct NineSlice2 {
    math::vec2 m_xy{};
    const LinearAtlas* m_atlas = nullptr;
    std::array<uint32_t, 9> m_spriteIds{};

    uint32_t m_currentVert = 0;
    std::array<float, 3> m_w{};
    std::array<float, 3> m_h{};

    NineSlice2( const math::vec4& xywh, const LinearAtlas*, const std::array<uint32_t, 9>& ) noexcept;
    static constexpr uint32_t count() noexcept { return 54u; }
    math::vec4 operator () () noexcept;
};

struct Vert6 {
    math::vec4 m_xywh{};
    math::vec4 m_uvwh{};
    uint32_t m_currentVert = 0;

    static constexpr uint32_t count() noexcept { return 6u; }
    math::vec4 operator () () noexcept;
};


}
