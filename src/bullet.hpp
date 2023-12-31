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

struct Bullet {
public:
    enum class Type : uint8_t {
        eTorpedo,
        eBlaster,
        eDead,
    };

    math::vec3 m_position{};
    math::vec3 m_direction{};
    math::vec3 m_prevPosition{};
    std::array<math::vec3, 4> m_tail{};
    math::vec4 m_color1{};
    math::vec4 m_color2{};
    const SAObject* m_target = nullptr;
    float m_speed = 0.0f;
    float m_travelDistance = 0.0f;
    float m_size = 0.0f;
    uint16_t m_score = 0;
    uint16_t m_collideId = 0;
    uint8_t m_damage = 0;
    Type m_type{};

    Bullet() noexcept = default;
    Bullet( const WeaponCreateInfo&, const math::vec3& position, const math::vec3& direction );

    static void updateAll( const UpdateContext&, std::span<Bullet> );
    static void renderAll( const RenderContext&, std::span<Bullet>, Texture );
};

struct WeaponCreateInfo {
    math::vec4 color1{};
    math::vec4 color2{};
    float delay = 0.0f;
    float speed = 0.0f;
    float size = 2.6_m;
    uint16_t score_per_hit = 0;
    uint16_t uvid{};
    uint8_t damage = 0;
    Bullet::Type type{};
    Hash::value_type displayName{};
};
