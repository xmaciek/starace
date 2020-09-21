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
    double m_maxRange = 0.0;
    double m_range = 0.0;
    double m_rotX = 0.0;
    double m_rotY = 0.0;
    double m_rotZ = 0.0;
    double m_damage = 0.0;
    uint32_t m_rotation = 0;
    float m_color1[ 4 ]{};
    float m_color2[ 4 ]{};
    Type m_type = Type::eSlug;

    virtual Vertex collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;
    void draw1() const;
    void draw2() const;
    void drawLaser() const;

public:
    virtual ~Bullet() = default;
    explicit Bullet( const BulletProto& bp );

    uint32_t damage() const;
    Type type() const;
    virtual void draw() const override;
    virtual void processCollision( SAObject* object ) override;
    void setDirection( const Vertex& v );
    void update();

};

struct BulletProto {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double speed = 0.0;
    double damage = 0.0;
    double delay = 0.0;
    Bullet::Type type = Bullet::Type::eBlaster;
    uint32_t texture1 = 0;
    uint32_t texture2 = 0;
    uint32_t energy = 0;
    uint32_t score_per_hit = 0;
    float color1[ 4 ]{};
    float color2[ 4 ]{};
};
