#pragma once

#include "saobject.hpp"
#include "units.hpp"
#include "explosion.hpp"

#include <audio/audio.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/buffer.hpp>
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
    Signal m_target{};
    Buffer m_mesh{};
    Texture m_texture{};
    float m_speed = 0.0f;
    float m_travelDistance = 0.0f;
    float m_maxDistance = 0.0f;
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
    Signal target{};
    Buffer mesh{};
    Texture texture{};
    float delay = 0.0f;
    float speed = 0.0f;
    float distance = 0.0;
    float reload = 0.001f;
    uint16_t capacity = 0xFFFF;
    uint16_t score_per_hit = 0;
    uint8_t damage = 0;
    Bullet::Type type{};
    Hash::value_type displayIcon{};
    Hash::value_type displayName{};
    Audio::Slot sound{};
};
