#pragma once

#include "sa.hpp"
#include "saobject.hpp"
#include "tail.hpp"

struct BulletProto {
    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;
    GLdouble speed = 0.0;
    GLdouble damage = 0.0;
    GLdouble delay = 0.0;
    GLuint type = 0;
    GLuint texture1 = 0;
    GLuint texture2 = 0;
    GLuint energy = 0;
    GLuint score_per_hit = 0;
    GLfloat color1[ 4 ]{};
    GLfloat color2[ 4 ]{};
};

class Bullet : public SAObject {
private:
    Tail m_tail{};
    GLdouble m_maxRange = 0.0;
    GLdouble m_range = 0.0;
    GLdouble m_rotX = 0.0;
    GLdouble m_rotY = 0.0;
    GLdouble m_rotZ = 0.0;
    GLdouble m_damage = 0.0;
    GLuint m_type = 0;
    GLuint m_rotation = 0;
    GLfloat m_color1[ 4 ]{};
    GLfloat m_color2[ 4 ]{};

    virtual Vertex collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;
    inline void Draw1() const;
    inline void Draw2() const;
    inline void DrawLaser() const;

public:
    explicit Bullet( const BulletProto& bp );
    virtual ~Bullet() = default;
    void SetDirection( Vertex v );
    virtual void Draw() const override;
    void Update();
    GLuint getDamage() const;
    GLuint GetType() const;

    virtual void ProcessCollision( SAObject* object ) override;

    static const GLuint SLUG = 0;
    static const GLuint BLASTER = 1;
    static const GLuint TORPEDO = 2;
    static const GLuint WAVE = 3;
    static const GLuint MINE = 4;
};
