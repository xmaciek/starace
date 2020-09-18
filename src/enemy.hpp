#ifndef SA_ENEMY_H
#define SA_ENEMY_H

#include "bullet.hpp"
#include "model.hpp"
#include "sa.hpp"
#include "saobject.hpp"
#include "shield.hpp"

class Enemy : public SAObject {
public:
    Enemy();
    ~Enemy();
    void Draw();
    void DrawCollisionIndicator();
    void Update();
    void SetModel( Model* M );
    void SetVisibleRange( const GLdouble& Range );
    void SetWeapon( const BulletProto& b );
    Bullet* GetWeapon();
    bool IsWeaponReady();
    void DrawRadarPosition( const Vertex& Modifier, const GLdouble& RadarScale );
    //   GLfloat getX();
    //   GLfloat getY();
    //   GLfloat getZ();

    virtual void ProcessCollision( SAObject* Object );

private:
    void ReinitCoordinates();
    //   GLfloat x,y,z;
    GLdouble visible_range, out_range;

    BulletProto weapon;
    GLdouble shotfactor;
    GLfloat health_perc;
    GLint shieldstrength;
    Model* model;
    Shield* shield;
};

#endif
