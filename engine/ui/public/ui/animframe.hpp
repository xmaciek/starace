#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>
#include <ui/sprite.hpp>
#include <shared/hash.hpp>
#include <renderer/pipeline.hpp>

#include <cstdint>

namespace ui {

class AnimFrame : public Widget {
    math::vec4 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<Sprite, 16> m_uvwh{};
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_revision = 0xFFFF;
    uint32_t m_index{};
    uint32_t m_count{};
    float m_spinner = 0.0f;
    PipelineSlot m_pipeline{};

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type data{};
        Hash::value_type color = "white"_hash;
        std::array<Hash::value_type, 16> frames{};
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    virtual ~AnimFrame() noexcept override = default;
    AnimFrame() noexcept = default;
    AnimFrame( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
    virtual void update( const UpdateContext& ) override;

};

}