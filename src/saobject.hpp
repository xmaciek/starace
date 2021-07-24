#pragma once

#include "render_context.hpp"
#include "update_context.hpp"

#include <glm/vec3.hpp>

#include <cstdint>

class SAObject {
public:
    enum struct Status {
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
    double collisionDamage() const;
    double collisionDistance() const;
    double health() const;
    float speed() const;
    float x() const;
    float y() const;
    float z() const;
    glm::vec3 direction() const;
    glm::vec3 position() const;
    glm::vec3 velocity() const;
    int32_t score() const;
    virtual void addScore( int32_t s, bool b );
    virtual void processCollision( SAObject* );
    virtual void render( RenderContext ) const = 0;
    virtual void update( const UpdateContext& ) = 0;
    void kill();
    void setDamage( double d );
    void setStatus( Status s );
    void setTarget( SAObject* t );
    void targetMe( bool );

protected:
    SAObject* m_target = nullptr;
    double m_collisionDamage = 0.0;
    double m_collisionDistance = 0.0;
    double m_health = 0.0;
    double m_turnrate = 0;
    float m_speed = 0.0;
    glm::vec3 m_direction{};
    glm::vec3 m_position{};
    glm::vec3 m_velocity{};
    uint32_t m_score = 0;
    uint32_t m_ttl = 0;
    Status m_status = Status::eDead;
    bool m_collisionFlag = false;
    bool m_isTargeted = false;

    void interceptTarget();
};
