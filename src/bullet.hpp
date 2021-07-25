#pragma once

#include "saobject.hpp"

#include <glm/vec4.hpp>

#include <cstdint>
#include <vector>

struct BulletProto;

class Bullet : public SAObject {
public:
    enum struct Type : uint8_t {
        eBlaster,
        eTorpedo,
        eSlug,
    };

private:
    std::vector<glm::vec3> m_tail{};
    glm::vec4 m_color1{};
    glm::vec4 m_color2{};
    float m_seankyDeltaTime = 0.0f;
    float m_maxRange = 150.0;
    float m_range = 0.0;
    uint16_t m_rotation = 0;
    uint8_t m_damage = 0;
    Type m_type = Type::eSlug;

    virtual glm::vec3 collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;

public:
    virtual ~Bullet() = default;
    explicit Bullet( const BulletProto& bp );

    uint32_t damage() const;
    Type type() const;
    virtual void processCollision( SAObject* object ) override;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setDirection( const glm::vec3& );
};

struct BulletProto {
    glm::vec4 color1{};
    glm::vec4 color2{};
    glm::vec3 position{};
    float delay = 0.0f;
    float speed = 0.0f;
    uint32_t energy = 0;
    uint32_t score_per_hit = 0;
    uint8_t damage = 0;
    Bullet::Type type = Bullet::Type::eBlaster;
};
