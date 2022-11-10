#pragma once

#include "saobject.hpp"
#include "units.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <shared/pmr_pointer.hpp>

#include <array>
#include <cstdint>
#include <vector>
#include <memory_resource>
#include <span>

struct WeaponCreateInfo;

class Bullet : public SAObject {
public:
    enum class Type : uint8_t {
        eBlaster,
        eTorpedo,
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
    Type m_type{};

public:
    virtual ~Bullet() override = default;
    explicit Bullet( const WeaponCreateInfo&, const math::vec3& position );

    uint8_t damage() const;
    Type type() const;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setDirection( const math::vec3& );

    math::vec3 prevPosition() const;
    uint16_t score() const;

    static void renderAll( const RenderContext&, std::span<const UniquePointer<Bullet>> );
};

struct WeaponCreateInfo {
    math::vec4 color1{};
    math::vec4 color2{};
    Texture texture{};
    float delay = 0.0f;
    float speed = 0.0f;
    uint16_t score_per_hit = 0;
    uint8_t damage = 0;
    Bullet::Type type{};
    Hash::value_type displayName{};
};
