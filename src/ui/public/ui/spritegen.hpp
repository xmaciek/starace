#pragma once

#include <ui/atlas.hpp>

#include <engine/math.hpp>

#include <array>
#include <cstdint>

namespace ui {

struct NineSlice2 {
    math::vec2 m_xy{};
    const Atlas* m_atlas = nullptr;
    std::array<Atlas::hash_type, 9> m_spriteIds{};

    uint32_t m_currentVert = 0;
    std::array<float, 3> m_w{};
    std::array<float, 3> m_h{};

    NineSlice2( const math::vec4& xywh, const Atlas*, const std::array<Atlas::hash_type, 9>& ) noexcept;
    static constexpr uint32_t count() noexcept { return 54u; }
    math::vec4 operator () () noexcept;
    math::vec4 operator () ( uint32_t spriteId ) const noexcept;
};

struct Vert6 {
    math::vec4 m_xywh{};
    math::vec4 m_uvwh{};
    uint32_t m_currentVert = 0;

    static constexpr uint32_t count() noexcept { return 6u; }
    math::vec4 operator () () noexcept;
};


}
