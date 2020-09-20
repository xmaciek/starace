#pragma once

#include "sa.hpp"
#include "saobject.hpp"
#include "tail.hpp"

struct BulletProto;

class Bullet : public SAObject {
public:
    enum struct Type {
        eBlaster,
        eTorpedo,
        eSlug,
    };

private:
    Tail m_tail{};
    GLdouble m_maxRange = 0.0;
    GLdouble m_range = 0.0;
    GLdouble m_rotX = 0.0;
    GLdouble m_rotY = 0.0;
    GLdouble m_rotZ = 0.0;
    GLdouble m_damage = 0.0;
    GLuint m_rotation = 0;
    GLfloat m_color1[ 4 ]{};
    GLfloat m_color2[ 4 ]{};
    Type m_type = Type::eSlug;

    virtual Vertex collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;
    void draw1() const;
    void draw2() const;
    void drawLaser() const;

public:
    virtual ~Bullet() = default;
    explicit Bullet( const BulletProto& bp );

    GLuint damage() const;
    Type type() const;
    virtual void draw() const override;
    virtual void processCollision( SAObject* object ) override;
    void setDirection( const Vertex& v );
    void update();

};

struct BulletProto {
    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;
    GLdouble speed = 0.0;
    GLdouble damage = 0.0;
    GLdouble delay = 0.0;
    Bullet::Type type = Bullet::Type::eBlaster;
    GLuint texture1 = 0;
    GLuint texture2 = 0;
    GLuint energy = 0;
    GLuint score_per_hit = 0;
    GLfloat color1[ 4 ]{};
    GLfloat color2[ 4 ]{};
};
