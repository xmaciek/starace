#pragma once

#include "widget.hpp"
#include "colors.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>

class UIImage : public Widget {
    math::vec4 m_color = color::white;
    Texture m_texture{};

public:
    UIImage() = default;
    UIImage( Texture t );
    UIImage( Texture t, math::vec4 color );
    UIImage( math::vec2 position, math::vec2 extent, math::vec4 color, Texture );
    UIImage( Anchor a, math::vec2 extent, math::vec4 color, Texture );
    UIImage( Anchor );

    virtual void render( RenderContext ) const override;
    void setColor( math::vec4 );
    void setTexture( Texture );
};
