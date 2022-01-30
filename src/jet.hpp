#pragma once

#include "bullet.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "reactor.hpp"
#include "saobject.hpp"
#include "shield.hpp"
#include "thruster.hpp"
#include "units.hpp"

#include <engine/math.hpp>

#include <vector>

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
    Thruster m_thruster;
    Shield m_shield;

    Model* m_model = nullptr;

    BulletProto m_weapon[ 3 ]{};

    math::quat m_animation{ math::vec3{} };
    math::quat m_quaternion{ math::vec3{} };

    // pitch yaw roll controlls
    math::vec3 m_pyrAccelleration{};
    math::vec3 m_pyrCurrent{};
    math::vec3 m_pyrLimits{};
    math::vec3 m_pyrTarget{};
    math::vec3 m_pyrAnimCurrent{};

    Reactor m_reactor{};

    float m_shotFactor[ 3 ]{};
    float m_speedMax = 3000_kmps;
    float m_speedMin = 320_kmps;
    float m_speedNorm = 1080_kmps;
    float m_speedTarget = 0.0f;
    float m_speedAcceleration = 430_kmps;

    Input m_input{};

public:
    virtual ~Jet() noexcept override = default;
    Jet() noexcept = default;
    explicit Jet( const ModelProto& ) noexcept;

    Bullet* weapon( uint32_t weaponNum, void* ptr );
    bool isShooting( uint32_t weaponNum ) const;
    bool isWeaponReady( uint32_t weaponNum ) const;
    double energy() const;
    math::quat animation() const;
    math::quat quat() const;
    math::quat rotation() const;
    math::vec3 weaponPoint( uint32_t weaponNum );
    virtual void addScore( uint32_t s, bool b ) override;
    virtual void processCollision( SAObject* ) override;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void lockTarget( SAObject* );
    void processCollision( std::vector<Bullet*>& );
    void setModel( Model* );
    void setWeapon( BulletProto bp, uint32_t id );
    void takeEnergy( uint32_t weaponNum );
    void untarget( const SAObject* );
    void setInput( const Input& );
};
