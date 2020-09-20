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
    Circle m_crosshair;
    Thruster m_thruster;
    Shield m_shield;

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
    virtual ~Jet() override = default;
    explicit Jet( const ModelProto& );

    Bullet* weapon( GLuint weaponNum );
    Quaternion animation() const;
    Quaternion quat() const;
    Quaternion rotation() const;
    Vertex weaponPoint( GLuint weaponNum );
    bool isShooting( GLuint weaponNum ) const;
    bool isWeaponReady( GLuint weaponNum ) const;
    double energy() const;
    virtual void addScore( GLint s, bool b ) override;
    virtual void draw() const override;
    virtual void processCollision( SAObject* ) override;
    void drawWireframe();
    void lockTarget( SAObject* );
    void pitchDown( bool );
    void pitchUp( bool );
    void processCollision( std::vector<Bullet*>& );
    void rollLeft( bool );
    void rollRight( bool );
    void setModel( Model* );
    void setWeapon( BulletProto bp, GLuint id );
    void shoot( GLuint weaponNum, bool );
    void speedDown( bool );
    void speedUp( bool );
    void takeEnergy( GLuint weaponNum );
    void update();
    void yawLeft( bool );
    void yawRight( bool );
};
