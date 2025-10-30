#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/hash.hpp>

#include <cstdint>

namespace ui {

class Progressbar : public Widget {
    math::vec4 m_uvwh{};
    math::vec2 m_spriteSize{};
    ui::DataModel* m_dataModel = nullptr;
    ui::DataModel::size_type m_revision = 0xFFFF;
    Texture m_texture{};
    PipelineSlot m_pipeline{};
    float m_value = 0.0f;
    float m_spacing = 0.0f;
    uint32_t m_count = 0;

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{ 128.0f, 16.0f };
        Hash::value_type data{};
        Hash::value_type path{};
        float spriteSpacing{};
        uint32_t count = 0;
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    virtual ~Progressbar() noexcept override = default;
    Progressbar() noexcept = default;
    Progressbar( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
    virtual void update( const UpdateContext& ) override;

};

}
