#pragma once

#include "widget.hpp"

class Glow : public Widget
{
    math::vec4 m_color{};

public:
    ~Glow() noexcept = default;
    Glow() noexcept = default;
    inline Glow( math::vec4 color ) noexcept
    : m_color{ color }
    {}

    virtual void render( ui::RenderContext ) const override;
};
