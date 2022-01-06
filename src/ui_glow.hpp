#pragma once

#include "widget.hpp"

class Glow : public Widget
{
    glm::vec4 m_color{};

public:
    ~Glow() noexcept = default;
    Glow() noexcept = default;
    inline Glow( glm::vec4 color ) noexcept
    : m_color{ color }
    {}

    virtual void render( RenderContext ) const override;
};
