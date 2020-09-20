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

    GLdouble collisionDamage() const;
    GLdouble collisionDistance() const;
    GLdouble health() const;
    GLdouble speed() const;
    GLdouble x() const;
    GLdouble y() const;
    GLdouble z() const;
    GLint score() const;
    Status status() const;
    Vertex direction() const;
    Vertex position() const;
    Vertex velocity() const;
    bool canCollide() const;
    bool deleteMe();
    virtual void addScore( GLint s, bool b );
    virtual void draw() const = 0;
    virtual void processCollision( SAObject* );
    void kill();
    void setDamage( GLdouble d );
    void setStatus( Status s );
    void setTarget( SAObject* t );
    void targetMe( bool );

protected:
    SAObject* m_target = nullptr;
    GLdouble m_collisionDamage = 0.0;
    GLdouble m_collisionDistance = 0.0;
    GLdouble m_health = 0.0;
    GLdouble m_speed = 0.0;
    GLdouble m_turnrate = 0;
    Vertex m_direction{};
    Vertex m_position{};
    Vertex m_velocity{};
    GLint m_score = 0;
    GLuint m_ttl = 0;
    Status m_status = Status::eDead;
    bool m_collisionFlag = false;
    bool m_isTargeted = false;

    void interceptTarget();
};
