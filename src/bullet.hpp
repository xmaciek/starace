#pragma once

#include "saobject.hpp"
#include "units.hpp"
#include "explosion.hpp"

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
        eLaser,
        eDead,
    };

    math::vec3 m_position{};
    math::vec3 m_direction{};
    math::quat m_quat{};
    math::vec3 m_prevPosition{};
    math::vec4 m_color1{};
    math::vec4 m_color2{};
    Signal m_target{};
    float m_speed = 0.0f;
    float m_travelDistance = 0.0f;
    float m_maxDistance = 0.0f;
    float m_size = 0.0f;
    uint16_t m_score = 0;
    uint16_t m_collideId = 0;
    uint8_t m_damage = 0;
    Type m_type{};

    Bullet() noexcept = default;
    Bullet( const WeaponCreateInfo&, const math::vec3& position, const math::vec3& direction );

    static void updateAll( const UpdateContext&, std::span<Bullet>, std::pmr::vector<Explosion>&, Texture );
    static void renderAll( const RenderContext&, std::span<Bullet>, Texture );
    static void scanSignals( std::span<Bullet>, std::span<const Signal> );
};

struct WeaponCreateInfo {
    math::vec4 color1{};
    math::vec4 color2{};
    Signal target{};
    float delay = 0.0f;
    float speed = 0.0f;
    float distance = 0.0;
    float size = 2.6_m;
    float reload = 0.0f;
    uint16_t capacity = 0xFFFF;
    uint16_t score_per_hit = 0;
    uint16_t uvid{};
    uint8_t damage = 0;
    Bullet::Type type{};
    Hash::value_type displayName{};
};
