#pragma once

#include <ui/widget.hpp>
#include <ui/pipeline.hpp>
#include <ui/sprite.hpp>

#include <shared/hash.hpp>
#include <shared/stack_vector.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <array>
#include <optional>

namespace ui {

class Decorator : public Widget {
protected:
    PushConstant<Pipeline::e9Slice>::Instance m_data{};
    std::optional<math::vec4> m_color;
    PipelineSlot m_pipeline{};
    Texture m_texture{};

    math::vec4 color( const RenderContext& ) const;

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type style = "box"_hash;
        Hash::value_type path = "box"_hash;
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    ~Decorator() noexcept = default;
    Decorator() noexcept = default;
    Decorator( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;

    void setSprite( const Sprite&, float range = 0.0f );
};

}
