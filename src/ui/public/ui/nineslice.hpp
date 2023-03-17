#pragma once

#include <ui/widget.hpp>

#include <engine/math.hpp>
#include <renderer/texture.hpp>

namespace ui {
class LinearAtlas;

class NineSlice : public Widget {
protected:
    const LinearAtlas* m_atlas = nullptr;
    float m_top = 0.0f;
    float m_bot = 0.0f;
    float m_left = 0.0f;
    float m_right = 0.0f;
    Texture m_texture{};
    std::array<uint32_t, 9> m_spriteIds{};

public:
    NineSlice() noexcept = default;
    NineSlice( math::vec2 position, math::vec2 extent, Anchor, const LinearAtlas*, std::array<uint32_t, 9>, Texture ) noexcept;

    virtual void render( RenderContext ) const override;
};

}

using NineSlice = ui::NineSlice;
