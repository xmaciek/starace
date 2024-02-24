#pragma once

#include <ui/data_model.hpp>
#include <ui/widget.hpp>
#include <ui/pipeline.hpp>

#include <shared/hash.hpp>
#include <engine/math.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>

namespace ui {

class Image : public Widget {
    math::vec4 m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    math::vec4 m_uvwh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
    Texture m_texture{};
    ui::Pipeline m_pipeline{};
    PipelineSlot m_pipelineSlot{};
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_current = 0;

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        Texture texture{};
        Hash::value_type sprite{};
        math::vec2 position{};
        math::vec2 size{};
        math::vec4 color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
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
