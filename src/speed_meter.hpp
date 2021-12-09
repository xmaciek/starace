#pragma once

#include "label.hpp"

#include <glm/vec4.hpp>

#include <array>

class Font;

class SpeedMeter : public Widget {
    Label m_speed{};
    Label m_speedValue{};

    float m_speedFan = 0.0f;
    float m_speedFanAngle = 0.0f;

public:
    ~SpeedMeter() noexcept = default;
    SpeedMeter() noexcept = default;
    SpeedMeter( Font* ) noexcept;

    void setSpeed( float );

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
};
