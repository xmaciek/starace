#pragma once

#include "saobject.hpp"
#include "units.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>

#include <cstdint>
#include <vector>
#include <memory_resource>

struct BulletProto;

class Bullet : public SAObject {
public:
    enum struct Type : uint8_t {
        eBlaster,
        eTorpedo,
        eSlug,
    };

private:
    std::pmr::vector<math::vec3> m_tail{};
    math::vec4 m_color1{};
    math::vec4 m_color2{};
    Texture m_texture{};
    float m_tailChunkLength = 0.0f;
    float m_seankyDeltaTime = 0.0f;
    float m_maxRange = 6000.0_m;
    float m_range = 0.0;
    uint8_t m_damage = 0;
    Type m_type = Type::eSlug;

    virtual math::vec3 collisionRay() const;
    virtual bool collisionTest( const SAObject* object ) const;

public:
    virtual ~Bullet() = default;
    explicit Bullet( const BulletProto& bp );

    uint8_t damage() const;
    Type type() const;
    virtual void processCollision( SAObject* object ) override;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setDirection( const math::vec3& );
};

struct BulletProto {
    math::vec4 color1{};
    math::vec4 color2{};
    math::vec3 position{};
    Texture texture{};
    float delay = 0.0f;
    float speed = 0.0f;
    uint32_t energy = 0;
    uint32_t score_per_hit = 0;
    uint8_t damage = 0;
    Bullet::Type type = Bullet::Type::eBlaster;
};
