#pragma once

#include "widget.hpp"
#include "linear_atlas.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>

class NineSlice : public Widget {
    math::vec4 m_color{};
    const LinearAtlas* m_atlas = nullptr;
    float m_top = 0.0f;
    float m_bot = 0.0f;
    float m_left = 0.0f;
    float m_right = 0.0f;
    Texture m_texture{};
    std::array<uint32_t, 9> m_spriteIds{};


public:
    NineSlice() noexcept = default;
    NineSlice( math::vec2 position, math::vec2 extent, math::vec4 color, Anchor, const LinearAtlas*, std::array<uint32_t, 9>, Texture ) noexcept;

    virtual void render( RenderContext ) const override;
    void setColor( math::vec4 c );
};
