#ifndef SA_ENEMY_H
#define SA_ENEMY_H

#include "bullet.hpp"
#include "model.hpp"
#include "sa.hpp"
#include "saobject.hpp"
#include "shield.hpp"

class Enemy : public SAObject {
public:
    virtual ~Enemy() override = default;
    Enemy();
    void Draw();
    static void DrawCollisionIndicator();
    void Update();
    void SetModel( Model* M );
    void SetVisibleRange( const GLdouble& Range );
    void SetWeapon( const BulletProto& b );
    Bullet* GetWeapon();
    bool IsWeaponReady() const;
    void DrawRadarPosition( const Vertex& Modifier, const GLdouble& RadarScale );

    virtual void ProcessCollision( SAObject* Object ) override;

private:
    void ReinitCoordinates();
    GLdouble visible_range = 0.0;
    GLdouble out_range = 0.0;

    BulletProto weapon{};
    GLdouble shotfactor = 0.0;
    GLfloat health_perc = 0.0f;
    GLint shieldstrength = 0;
    Shield shield;
};

#endif
