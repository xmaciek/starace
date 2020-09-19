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
public:
    GLdouble energy = 0.0;
    Quaternion quaternion{};
    Quaternion rotation{};
    explicit Jet( const ModelProto& model_data );
    ~Jet();

    void Draw();
    void DrawWireframe();
    void Update();
    void RollLeft( bool doit );
    void RollRight( bool doit );
    void PitchUp( bool doit );
    void PitchDown( bool doit );
    void YawLeft( bool doit );
    void YawRight( bool doit );
    void SpeedUp( bool doit );
    void SpeedDown( bool doit );
    void SetModel( Model* M );

    void LockTarget( SAObject* t );
    bool IsWeaponReady( GLuint WeaponNum );
    bool IsShooting( GLuint WeaponNum );
    void Shoot( GLuint WeaponNum, bool doit );
    Vertex GetWeaponPoint( GLuint wID );
    Bullet* GetWeaponType( GLuint wID );
    void SetWeapon( BulletProto bp, GLuint ID );
    void TakeEnergy( GLuint wID );
    void ProcessCollision( std::vector<Bullet*>& Bullets );
    virtual void ProcessCollision( SAObject* ) override;
    virtual void AddScore( GLint s, bool b ) override;

private:
    GLdouble roll = 0.0;
    GLdouble pitch = 0.0;
    GLdouble yaw = 0.0;
    GLdouble rotZ = 0.0;
    GLdouble rotX = 0.0;
    GLdouble rotY = 0.0;
    BulletProto Weapon[ 3 ]{};
    GLint accX = 0;
    GLint accY = 0;
    GLint accZ = 0;
    GLint maxangleX = 0;
    GLint maxangleY = 0;
    GLint maxangleZ = 0;
    GLdouble anglespeedX = 0.0;
    GLdouble anglespeedY = 0.0;
    GLdouble anglespeedZ = 0.0;

    bool btn_roll_left = false;
    bool btn_roll_right = false;
    bool btn_yaw_left = false;
    bool btn_yaw_right = false;
    bool btn_pitch_up = false;
    bool btn_pitch_down = false;

    Model model{};

    GLbyte speed_acc = 0;
    GLdouble max_speed = 0.0;
    GLdouble min_speed = 0.0;
    GLdouble norm_speed = 0.0;

    Circle* crosshair = nullptr;
    Thruster* thruster = nullptr;
    Shield* shield = nullptr;
    Quaternion animation{};
    GLubyte CIRCLE_LOOP = 0;
    bool shooting[ 3 ]{};
    GLdouble shotfactor[ 3 ]{};
    bool target_locked[ 3 ]{};
};
