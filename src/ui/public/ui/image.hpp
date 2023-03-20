#pragma once

#include <ui/data_model.hpp>
#include <ui/widget.hpp>

#include <engine/math.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>

namespace ui {

class Image : public Widget {
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_current = 0;
    math::vec4 m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    Texture m_texture{};

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        Texture texture{};
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
