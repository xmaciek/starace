#pragma once

#include "autolerp.hpp"
#include "bullet.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "saobject.hpp"
#include "thruster.hpp"
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

    struct Input {
        float pitch = 0.0f;
        float yaw = 0.0f;
        float roll = 0.0f;
        float speed = 0.0f;
        bool shoot1 = false;
        bool shoot2 = false;
        bool shoot3 = false;
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
    Thruster m_thruster[ 2 ]{};
    AutoLerp<math::vec4> m_thrusterAngles{ {}, {}, 20.0_deg };
    AutoLerp<math::vec2> m_thrusterLength{ { 10.0_m, 10.0_m }, { 10.0_m, 10.0_m }, 10.0_m };

    Model m_model{};

    struct WeaponCooldown {
        float current;
        float ready;
    };

    std::array<WeaponCooldown, MAX_SUPPORTED_WEAPON_COUNT> m_weaponsCooldown{};
    std::array<WeaponCreateInfo, MAX_SUPPORTED_WEAPON_COUNT> m_weapons{};

    math::quat m_quaternion{ math::vec3{} };

    // pitch yaw roll controls
    math::vec3 m_pyrLimits{};

    AutoLerp<math::vec3> m_camOffset{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0.2f };
    AutoLerp<math::vec3, math::vec3> m_angleState{};

    float m_speedMax = 1800_kmph;
    float m_speedMin = 192_kmph;
    float m_speedNorm = 600_kmph;
    float m_accell = 256_kmph;
    float m_deaccell = 256_kmph;
    AutoLerp<> m_speedTarget{ 600_kmph, 600_kmph, 256_kmph };

    PointInfo m_points{};
    Input m_input{};
    bool m_vectorThrust = true;

    math::vec3 weaponPoint( uint32_t );
    bool isShooting( uint32_t ) const;
    UniquePointer<Bullet> weapon( uint32_t, std::pmr::memory_resource* );

public:
    virtual ~Jet() noexcept override = default;
    Jet() noexcept = default;
    Jet( const CreateInfo& ) noexcept;

    std::span<UniquePointer<Bullet>> shoot( std::pmr::memory_resource*, std::pmr::vector<UniquePointer<Bullet>>* );

    math::quat quat() const;
    math::quat rotation() const;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void lockTarget( SAObject* );
    void processCollision( std::vector<Bullet*>& );
    void setModel( Model* );
    void untarget( const SAObject* );
    void setInput( const Input& );

    float targetingState() const;
    math::vec3 cameraPosition() const;
    math::vec3 cameraDirection() const;
};
