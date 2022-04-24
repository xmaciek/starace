#pragma once

#include "label.hpp"

class SpeedMeter : public Widget {
    Label m_speed{};
    Label m_speedValue{};

    float m_speedFan = 0.0f;
    float m_speedFanAngle = 0.0f;

public:
    ~SpeedMeter() noexcept = default;
    SpeedMeter() noexcept = default;
    SpeedMeter( std::nullptr_t ) noexcept;

    void setSpeed( float );

    virtual void render( ui::RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
};
