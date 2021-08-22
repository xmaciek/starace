#pragma once

#include "bullet.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "reactor.hpp"
#include "saobject.hpp"
#include "shield.hpp"
#include "thruster.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

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

    Model m_model{};

    BulletProto m_weapon[ 3 ]{};

    glm::quat m_animation{ glm::vec3{} };
    glm::quat m_quaternion{ glm::vec3{} };

    // pitch yaw roll controlls
    glm::vec3 m_pyrAccelleration{};
    glm::vec3 m_pyrCurrent{};
    glm::vec3 m_pyrLimits{};
    glm::vec3 m_pyrTarget{};
    glm::vec3 m_pyrAnimCurrent{};

    Reactor m_reactor{};

    float m_shotFactor[ 3 ]{};
    float m_speedMax = 5.0f;
    float m_speedMin = 0.5f;
    float m_speedNorm = 2.0f;
    float m_speedTarget = 0.0f;
    float m_speedAcceleration = 0.8f;

    int32_t m_maxAngleX = 5;
    int32_t m_maxAngleY = 5;
    int32_t m_maxAngleZ = 15;

    Input m_input{};

public:
    virtual ~Jet() override = default;
    explicit Jet( const ModelProto& );

    Bullet* weapon( uint32_t weaponNum, void* ptr );
    bool isShooting( uint32_t weaponNum ) const;
    bool isWeaponReady( uint32_t weaponNum ) const;
    double energy() const;
    glm::quat animation() const;
    glm::quat quat() const;
    glm::quat rotation() const;
    glm::vec3 weaponPoint( uint32_t weaponNum );
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
