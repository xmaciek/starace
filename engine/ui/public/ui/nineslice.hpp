#pragma once

#include <ui/widget.hpp>
#include <ui/pipeline.hpp>

#include <shared/hash.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>

#include <array>

namespace ui {

class NineSlice : public Widget {
protected:
    using Uniform = PushConstant<Pipeline::eSpriteSequence>;
    std::array<Uniform::Sprite, 9> m_sprites{};
    std::array<Texture, 9> m_textures{};

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
