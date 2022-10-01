#pragma once

#include "bullet.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "saobject.hpp"
#include "thruster.hpp"
#include "units.hpp"
#include "chase.hpp"

#include <shared/pmr_pointer.hpp>

#include <engine/math.hpp>

#include <vector>
#include <memory_resource>

class Renderer;
class Jet : public SAObject {
public:
    struct Input {
        float pitch = 0.0f;
        float yaw = 0.0f;
        float roll = 0.0f;
        float speed = 0.0f;
        bool shoot1 = false;
        bool shoot2 = false;
        bool shoot3 = false;
    };

private:
    Thruster m_thruster[ 2 ]{};
    Chase<math::vec2> m_vectorThrustLeft{ {}, {}, 20.0_deg };
    Chase<math::vec2> m_vectorThrustRight{ {}, {}, 20.0_deg };
    Chase<math::vec2> m_thrusterLength{ { 10.0_m, 10.0_m }, { 10.0_m, 10.0_m }, 10.0_m };

    Model m_model{};

    WeaponCreateInfo m_weapon[ 3 ]{};
    float m_weaponCooldown[ 3 ]{};

    math::quat m_quaternion{ math::vec3{} };

    // pitch yaw roll controls
    math::vec3 m_pyrLimits{};

    Chase<math::vec3> m_camDirection{ { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, 0.2f };
    Chase<math::vec3> m_camPosition{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0.2f };
    Chase<math::vec3, math::vec3> m_angleState{ {}, {}, { 60.0_deg, 40.0_deg, 200.0_deg } };

    float m_speedMax = 1800_kmph;
    float m_speedMin = 192_kmph;
    float m_speedNorm = 600_kmph;
    // float m_speedAcceleration = 256_kmph;
    Chase<> m_speedTarget{ 600_kmph, 600_kmph, 256_kmph };


    Input m_input{};
    bool m_vectorThrust = true;

public:
    virtual ~Jet() noexcept override = default;
    Jet() noexcept = default;
    explicit Jet( const ModelProto& ) noexcept;

    UniquePointer<Bullet> weapon( uint32_t weaponNum, std::pmr::memory_resource* );
    bool isShooting( uint32_t weaponNum ) const;
    bool isWeaponReady( uint32_t weaponNum ) const;

    math::quat quat() const;
    math::quat rotation() const;
    math::vec3 weaponPoint( uint32_t weaponNum );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void lockTarget( SAObject* );
    void processCollision( std::vector<Bullet*>& );
    void setModel( Model* );
    void setWeapon( const WeaponCreateInfo&, uint32_t id );
    void untarget( const SAObject* );
    void setInput( const Input& );

    math::vec3 cameraPosition() const;
    math::vec3 cameraDirection() const;
};
