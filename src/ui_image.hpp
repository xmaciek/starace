#pragma once

#include "widget.hpp"
#include "colors.hpp"
#include "ui_data_model.hpp"

#include <engine/math.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>

namespace ui {

class Image : public Widget {
    DataModel* m_dataModel = nullptr;
    DataModel::size_type m_current = 0;
    math::vec4 m_color = color::white;
    Texture m_texture{};

public:
    Image() = default;
    Image( Texture t );
    Image( Texture t, math::vec4 color );
    Image( math::vec2 position, math::vec2 extent, math::vec4 color, Texture );
    Image( Anchor a, math::vec2 extent, math::vec4 color, Texture );
    Image( Anchor );
    Image( math::vec2 position, math::vec2 extent, DataModel* );

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setColor( math::vec4 );
    void setTexture( Texture );
};
}

using UIImage = ui::Image;
