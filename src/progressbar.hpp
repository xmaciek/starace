#pragma once

#include "widget.hpp"

class ProgressBar : public Widget {
    glm::vec4 m_colorA{};
    glm::vec4 m_colorB{};
    glm::vec2 m_axis{};
    float m_value = 0.0f;

public:
    virtual ~ProgressBar() noexcept override = default;
    ProgressBar() noexcept = default;
    ProgressBar( glm::vec2 pos, glm::vec2 size, glm::vec2 axis, const glm::vec4 colorA, const glm::vec4 colorB ) noexcept;

    void setValue( float );

    virtual void render( RenderContext ) const override;
};
