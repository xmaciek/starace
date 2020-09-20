#pragma once

#include "bullet.hpp"
#include "circle.hpp"
#include "model.hpp"
#include "quaternion.hpp"
#include "sa.hpp"
#include "saobject.hpp"
#include "shield.hpp"
#include "texture.hpp"
#include "thruster.hpp"

class Jet : public SAObject {
private:
    Circle* m_crosshair = nullptr;
    Thruster* m_thruster = nullptr;
    Shield* m_shield = nullptr;

    Model m_model{};

    BulletProto m_weapon[ 3 ]{};

    Quaternion m_animation{};
    Quaternion m_quaternion{};
    Quaternion m_rotation{};

    GLdouble m_angleSpeedX = 15.0;
    GLdouble m_angleSpeedY = 10.0;
    GLdouble m_angleSpeedZ = 50.0;
    GLdouble m_energy = 100.0;
    GLdouble m_maxSpeed = 5.0;
    GLdouble m_minSpeed = 0.5;
    GLdouble m_normSpeed = 2.0;
    GLdouble m_pitch = 0.0;
    GLdouble m_roll = 0.0;
    GLdouble m_rotX = 0.0;
    GLdouble m_rotY = 0.0;
    GLdouble m_rotZ = 0.0;
    GLdouble m_shotFactor[ 3 ]{};
    GLdouble m_yaw = 0.0;

    GLint m_accX = 0;
    GLint m_accY = 0;
    GLint m_accZ = 0;
    GLint m_maxAngleX = 5;
    GLint m_maxAngleY = 5;
    GLint m_maxAngleZ = 15;

    bool m_btnPitchDown = false;
    bool m_btnPitchUp = false;
    bool m_btnRollLeft = false;
    bool m_btnRollRight = false;
    bool m_btnYawLeft = false;
    bool m_btnYawRight = false;
    bool m_shooting[ 3 ]{};
    bool m_targetLocked[ 3 ]{};
    GLubyte m_circleLoop = 0;
    GLbyte m_speedAcc = 0;

public:
    virtual ~Jet() override;
    explicit Jet( const ModelProto& model_data );

    Bullet* GetWeaponType( GLuint wID );
    Vertex GetWeaponPoint( GLuint wID );
    bool IsShooting( GLuint WeaponNum );
    bool IsWeaponReady( GLuint WeaponNum );
    virtual void AddScore( GLint s, bool b ) override;
    virtual void Draw() const override;
    virtual void ProcessCollision( SAObject* ) override;
    void DrawWireframe();
    void LockTarget( SAObject* t );
    void PitchDown( bool doit );
    void PitchUp( bool doit );
    void ProcessCollision( std::vector<Bullet*>& Bullets );
    void RollLeft( bool doit );
    void RollRight( bool doit );
    void SetModel( Model* M );
    void SetWeapon( BulletProto bp, GLuint ID );
    void Shoot( GLuint WeaponNum, bool doit );
    void SpeedDown( bool doit );
    void SpeedUp( bool doit );
    void TakeEnergy( GLuint wID );
    void Update();
    void YawLeft( bool doit );
    void YawRight( bool doit );
    Quaternion rotation() const;
    Quaternion quat() const;
    Quaternion animation() const;
    double energy() const;
};
