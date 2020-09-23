#pragma once

#include "bullet.hpp"
#include "circle.hpp"
#include "model.hpp"
#include "model_proto.hpp"
#include "saobject.hpp"
#include "shield.hpp"
#include "thruster.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

class Jet : public SAObject {
private:
    Circle m_crosshair;
    Thruster m_thruster;
    Shield m_shield;

    Model m_model{};

    BulletProto m_weapon[ 3 ]{};

    glm::quat m_animation{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::quat m_quaternion{ 0.0f, 0.0f, 0.0f, 1.0f };
    glm::quat m_rotation{ 0.0f, 0.0f, 0.0f, 1.0f };

    double m_energy = 100.0;
    double m_pitch = 0.0;
    double m_roll = 0.0;
    double m_shotFactor[ 3 ]{};
    double m_yaw = 0.0;
    float m_angleSpeedX = 15.0;
    float m_angleSpeedY = 10.0;
    float m_angleSpeedZ = 50.0;
    float m_maxSpeed = 5.0f;
    float m_minSpeed = 0.5f;
    float m_normSpeed = 2.0f;
    float m_rotX = 0.0;
    float m_rotY = 0.0;
    float m_rotZ = 0.0;

    int32_t m_accX = 0;
    int32_t m_accY = 0;
    int32_t m_accZ = 0;
    int32_t m_maxAngleX = 5;
    int32_t m_maxAngleY = 5;
    int32_t m_maxAngleZ = 15;

    bool m_btnPitchDown = false;
    bool m_btnPitchUp = false;
    bool m_btnRollLeft = false;
    bool m_btnRollRight = false;
    bool m_btnYawLeft = false;
    bool m_btnYawRight = false;
    bool m_shooting[ 3 ]{};
    bool m_targetLocked[ 3 ]{};
    uint8_t m_circleLoop = 0;
    int8_t m_speedAcc = 0;

public:
    virtual ~Jet() override = default;
    explicit Jet( const ModelProto& );

    Bullet* weapon( uint32_t weaponNum );
    bool isShooting( uint32_t weaponNum ) const;
    bool isWeaponReady( uint32_t weaponNum ) const;
    double energy() const;
    glm::quat animation() const;
    glm::quat quat() const;
    glm::quat rotation() const;
    glm::vec3 weaponPoint( uint32_t weaponNum );
    virtual void addScore( int32_t s, bool b ) override;
    virtual void draw() const override;
    virtual void processCollision( SAObject* ) override;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void drawWireframe();
    void lockTarget( SAObject* );
    void pitchDown( bool );
    void pitchUp( bool );
    void processCollision( std::vector<Bullet*>& );
    void rollLeft( bool );
    void rollRight( bool );
    void setModel( Model* );
    void setWeapon( BulletProto bp, uint32_t id );
    void shoot( uint32_t weaponNum, bool );
    void speedDown( bool );
    void speedUp( bool );
    void takeEnergy( uint32_t weaponNum );
    void yawLeft( bool );
    void yawRight( bool );
};
