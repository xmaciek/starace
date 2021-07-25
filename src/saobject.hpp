#pragma once

#include "render_context.hpp"
#include "update_context.hpp"

#include <glm/vec3.hpp>

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
    bool canCollide() const;
    bool deleteMe();
    uint8_t collisionDamage() const;
    double collisionDistance() const;
    uint8_t health() const;
    float speed() const;
    float x() const;
    float y() const;
    float z() const;
    glm::vec3 direction() const;
    glm::vec3 position() const;
    glm::vec3 velocity() const;
    uint32_t score() const;
    virtual void addScore( uint32_t s, bool b );
    virtual void processCollision( SAObject* );
    virtual void render( RenderContext ) const = 0;
    virtual void update( const UpdateContext& ) = 0;
    void kill();
    void setDamage( uint8_t d );
    void setStatus( Status s );
    void setTarget( SAObject* t );
    void targetMe( bool );

protected:
    SAObject* m_target = nullptr;
    glm::vec3 m_direction{};
    glm::vec3 m_position{};
    glm::vec3 m_velocity{};
    float m_collisionDistance = 0.0f;
    float m_turnrate = 0.0f;
    float m_speed = 0.0f;
    uint32_t m_score = 0;
    uint8_t m_health = 0;
    uint8_t m_collisionDamage = 0;
    Status m_status = Status::eDead;
    bool m_collisionFlag = false;
    bool m_isTargeted = false;

    void interceptTarget();
};
