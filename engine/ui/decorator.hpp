#pragma once

#include <ui/widget.hpp>
#include <ui/pipeline.hpp>

#include <shared/hash.hpp>
#include <shared/stack_vector.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <array>

namespace ui {

class Decorator : public Widget {
protected:
    using Uniform = PushConstant<Pipeline::eSpriteSequence>;
    PipelineSlot m_pipeline{};
    Hash::value_type m_style{};
    StackVector<Texture, 9> m_textures{ 9u };
    StackVector<Uniform::Sprite, 9> m_sprites{ 9 };

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type style = "box"_hash;
        Hash::value_type path = "white"_hash;
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    ~Decorator() noexcept = default;
    Decorator() noexcept = default;
    Decorator( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
};

}
