#pragma once

#include <engine/math.hpp>

#include <array>
#include <cstdint>

class LinearAtlas;

namespace ui::generator {

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



}
