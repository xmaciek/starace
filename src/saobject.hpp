#pragma once

#include "sa.hpp"

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

    double collisionDamage() const;
    double collisionDistance() const;
    double health() const;
    double speed() const;
    double x() const;
    double y() const;
    double z() const;
    int32_t score() const;
    Status status() const;
    Vertex direction() const;
    Vertex position() const;
    Vertex velocity() const;
    bool canCollide() const;
    bool deleteMe();
    virtual void addScore( int32_t s, bool b );
    virtual void draw() const = 0;
    virtual void processCollision( SAObject* );
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
    double m_speed = 0.0;
    double m_turnrate = 0;
    Vertex m_direction{};
    Vertex m_position{};
    Vertex m_velocity{};
    int32_t m_score = 0;
    uint32_t m_ttl = 0;
    Status m_status = Status::eDead;
    bool m_collisionFlag = false;
    bool m_isTargeted = false;

    void interceptTarget();
};
