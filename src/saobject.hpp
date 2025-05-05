#pragma once

#include "render_context.hpp"
#include "update_context.hpp"
#include "signal.hpp"

#include <engine/math.hpp>

#include <cstdint>
#include <optional>
#include <span>

class SAObject {
public:
    enum struct Status : uint8_t {
        eDead,
        eAlive,
        eOut,
        eInvisible,
    };

    virtual ~SAObject() = default;
    SAObject() = default;

    Status status() const;
    uint16_t health() const;
    float speed() const;
    math::vec3 direction() const;
    math::vec3 position() const;
    math::vec3 velocity() const;
    void kill();
    void setDamage( uint8_t d );
    void setStatus( Status s );
    void setPosition( const math::vec3& );
    virtual void setTarget( Signal );

    static inline float pointsToMultiplier( uint8_t point ) noexcept
    {
        const float p = static_cast<float>( point ) * 0.05f;
        return 1.0f + p / ( 1.0f + p );
    }

    static Signal scanSignals( math::vec3 pos, std::span<const Signal> );

protected:
    math::vec3 m_direction{};
    math::vec3 m_position{};
    Signal m_target{};
    float m_speed = 0.0f;
    uint16_t m_pendingDamage = 0;
    uint16_t m_health = 0;
    Status m_status = Status::eDead;

};
