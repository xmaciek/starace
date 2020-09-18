#ifndef SA_TARGET_H
#define SA_TARGET_H

#include "sa.hpp"

class SAObject {
public:
    SAObject();
    virtual ~SAObject();
    GLdouble getX() const;
    GLdouble getY() const;
    GLdouble getZ() const;
    GLuint GetStatus() const;
    void SetStatus( const GLuint& s );
    void SetTarget( SAObject* t );
    Vertex GetPosition() const;
    Vertex GetDirection() const;
    Vertex GetVelocity() const;
    GLdouble GetSpeed() const;
    bool DeleteMe();
    void Kill();
    void Damage( const GLdouble& d );
    void TargetMe( const bool& doit );
    GLdouble GetHealth();

    virtual void ProcessCollision( SAObject* Object );
    bool CanCollide() const;
    GLdouble GetCollisionDistance() const;

    GLint GetScore();
    virtual void AddScore( const GLint& s, bool b = false );

    GLdouble GetCollisionDamage() const;

    static const GLuint DEAD = 0;
    static const GLuint ALIVE = 1;
    static const GLuint OUT = 2;
    static const GLuint NOT_VISIBLE = 3;

protected:
    SAObject* target = nullptr;
    bool ImTargeted = false;
    bool CollisionFlag = false;
    GLdouble CollisionDistance = 0.0;
    GLdouble CollisionDamage = 0.0;
    GLint score = 0;

    GLdouble speed = 0;
    GLuint status = 0;
    GLdouble health = 0.0;

    Vertex position{};
    Vertex direction{};
    Vertex velocity{};

    GLuint ttl = 0;

    GLdouble turnrate_in_rads = 0;
    void InterceptTarget();

};

#endif
