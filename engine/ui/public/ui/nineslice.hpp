#pragma once

#include <ui/widget.hpp>

#include <shared/hash.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>

namespace ui {

class NineSlice : public Widget {
public:
    using SpriteArray = std::array<Hash::value_type, 9>;

protected:
    float m_top = 0.0f;
    float m_bot = 0.0f;
    float m_left = 0.0f;
    float m_right = 0.0f;
    SpriteArray m_spriteIds{};

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type style = "box"_hash;
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    ~NineSlice() noexcept = default;
    NineSlice() noexcept = default;
    NineSlice( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
};

}
