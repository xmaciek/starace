#pragma once

#include <ui/data_model.hpp>
#include <ui/widget.hpp>
#include <ui/pipeline.hpp>

#include <shared/hash.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>

namespace ui {

class Image : public Widget {
    math::vec4 m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    math::vec4 m_uvwh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
    Texture m_texture{};
    uint32_t m_sampleRGBA = 0;
    PipelineSlot m_pipelineSlot{};
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_revision = 0xFFFF;

public:
    struct CreateInfo {
        Texture texture{};
        Hash::value_type data{};
        Hash::value_type spriteId{};
        Hash::value_type color = "white"_hash;
        math::vec2 position{};
        math::vec2 size{};
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };
    ~Image() noexcept = default;
    Image() noexcept = default;
    Image( const CreateInfo& ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setColor( math::vec4 );
    void setTexture( Texture );
};
}

using UIImage = ui::Image; // TODO remove
