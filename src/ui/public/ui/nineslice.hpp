#pragma once

#include <ui/atlas.hpp>
#include <ui/widget.hpp>

#include <engine/math.hpp>
#include <renderer/texture.hpp>

namespace ui {

class NineSlice : public Widget {
protected:
    float m_top = 0.0f;
    float m_bot = 0.0f;
    float m_left = 0.0f;
    float m_right = 0.0f;
    using SpriteArray = std::array<Atlas::hash_type, 9>;
    SpriteArray m_spriteIds{};

public:
    NineSlice() noexcept = default;
    NineSlice( math::vec2 position, math::vec2 extent, Anchor, SpriteArray) noexcept;

    virtual void render( RenderContext ) const override;
};

}

using NineSlice = ui::NineSlice;
