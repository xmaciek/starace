#pragma once

#include "autolerp.hpp"
#include "bullet.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "saobject.hpp"
#include "units.hpp"

#include <shared/pmr_pointer.hpp>

#include <engine/math.hpp>

#include <vector>
#include <memory_resource>
#include <span>

class Renderer;
class Jet : public SAObject {
public:
    static constexpr inline uint32_t MAX_SUPPORTED_WEAPON_COUNT = 3;
    static constexpr inline uint16_t COLLIDE_ID = 'JT';

    struct Input {
        float pitch = 0.0f;
        float yaw = 0.0f;
        float roll = 0.0f;
        float speed = 0.0f;
        bool shoot1 = false;
        bool shoot2 = false;
        bool lookAt = false;
    };
    struct PointInfo {
        uint8_t hp = 0;
        uint8_t armor = 0;
        uint8_t pitch = 0;
        uint8_t yaw = 0;
        uint8_t roll = 0;
        uint8_t speedMax = 0;
        uint8_t accell = 0;
        uint8_t deaccell = 0;
    };

    struct CreateInfo {
        Model model{};
        float modelScale = 1.0f;
        bool vectorThrust = false;
        PointInfo points{};
        std::array<WeaponCreateInfo, MAX_SUPPORTED_WEAPON_COUNT> weapons{};
    };

private:
    Model m_model{};

    struct WeaponCooldown {
        float currentDelay;
        float readyDelay;
        float currentReload;
        float readyReload;
        uint16_t count;
        uint16_t capacity;
    };

    std::array<WeaponCooldown, MAX_SUPPORTED_WEAPON_COUNT> m_weaponsCooldown{};
    std::array<WeaponCreateInfo, MAX_SUPPORTED_WEAPON_COUNT> m_weapons{};

    math::quat m_quaternion{ math::vec3{} };

    // pitch yaw roll controls
    math::vec3 m_pyrLimits{};

    AutoLerp<math::vec3> m_camOffset{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0.2f };
    AutoLerp<math::vec3, math::vec3> m_angleState{};

    float m_speedCurveMax = 1600_kmph;
    float m_speedCurveNorm = 300_kmph;
    float m_speedCurveMin = 200_kmph;
    float m_accell = 256_kmph;
    float m_deaccell = 256_kmph;
    AutoLerp<> m_speedTarget{};

    PointInfo m_points{};
    Input m_input{};

    Signal m_targetSignal{};
    math::vec3 m_targetVelocity{};

    math::vec3 weaponPoint( uint32_t ) const;
    bool isShooting( uint32_t ) const;
    Bullet weapon( uint32_t );

public:
    virtual ~Jet() noexcept override = default;
    Jet() noexcept = default;
    Jet( const CreateInfo& ) noexcept;

    std::array<Bullet::Type, MAX_SUPPORTED_WEAPON_COUNT> shoot( std::pmr::vector<Bullet>& );

    math::quat quat() const;
    math::quat rotation() const;
    void render( RenderContext ) const;
    void update( const UpdateContext& );
    void lockTarget( SAObject* );
    void processCollision( std::vector<Bullet*>& );
    void untarget( const SAObject* );
    void setInput( const Input& );
    void scanSignals( std::span<const Signal>, float dt );
    void setTarget( Signal );

    float targetingState() const;
    inline Signal targetSignal() const { return m_targetSignal; }

    math::vec3 cameraPosition() const;
    math::vec3 cameraDirection() const;
    math::vec2 reloadState() const;
    std::tuple<uint32_t, uint32_t> weaponClip() const;
};
