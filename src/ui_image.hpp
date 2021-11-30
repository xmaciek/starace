#pragma once

#include "widget.hpp"
#include "colors.hpp"

#include <renderer/texture.hpp>

#include <glm/vec4.hpp>

class UIImage : public Widget {
    glm::vec4 m_color = color::white;
    Texture m_texture{};

public:
    UIImage() = default;
    UIImage( Texture t );
    UIImage( glm::vec2 position, glm::vec2 extent, glm::vec4 color, Texture );

    virtual void render( RenderContext ) const override;
    void setColor( glm::vec4 );
};
