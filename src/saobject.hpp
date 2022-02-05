#pragma once

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

#include <cstdint>

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
    uint8_t health() const;
    float speed() const;
    math::vec3 direction() const;
    math::vec3 position() const;
    math::vec3 velocity() const;
    uint32_t score() const;
    virtual void addScore( uint32_t s, bool b );
    virtual void render( RenderContext ) const = 0;
    virtual void update( const UpdateContext& );
    void kill();
    void setDamage( uint8_t d );
    void setStatus( Status s );
    void setTarget( SAObject* t );
    SAObject* target() const;

protected:
    SAObject* m_target = nullptr;
    math::vec3 m_direction{};
    math::vec3 m_position{};
    float m_speed = 0.0f;
    uint32_t m_score = 0;
    uint16_t m_pendingDamage = 0;
    uint8_t m_health = 0;
    Status m_status = Status::eDead;

};
