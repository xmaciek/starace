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
    math::vec3 m_prevPosition{};
    std::array<math::vec3, 4> m_tail{};
    math::vec4 m_color1{};
    math::vec4 m_color2{};
    Texture m_texture{};
    float m_range = 0.0;
    uint16_t m_score = 0;
    uint8_t m_damage = 0;
    Type m_type = Type::eSlug;

public:
    virtual ~Bullet() override = default;
    explicit Bullet( const BulletProto& bp );

    uint8_t damage() const;
    Type type() const;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setDirection( const math::vec3& );

    math::vec3 prevPosition() const;
    uint16_t score() const;
};

struct BulletProto {
    math::vec4 color1{};
    math::vec4 color2{};
    math::vec3 position{};
    Texture texture{};
    float delay = 0.0f;
    float speed = 0.0f;
    uint32_t energy = 0;
    uint16_t score_per_hit = 0;
    uint8_t damage = 0;
    Bullet::Type type = Bullet::Type::eBlaster;
};
