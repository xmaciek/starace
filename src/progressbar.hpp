#pragma once

#include "widget.hpp"

class ProgressBar : public Widget {
    math::vec4 m_colorA{};
    math::vec4 m_colorB{};
    math::vec2 m_axis{};
    float m_value = 0.0f;

public:
    virtual ~ProgressBar() noexcept override = default;
    ProgressBar() noexcept = default;
    ProgressBar( math::vec2 pos, math::vec2 size, math::vec2 axis, const math::vec4 colorA, const math::vec4 colorB ) noexcept;

    void setValue( float );

    virtual void render( ui::RenderContext ) const override;
};
