#ifndef SA_ENEMY_H
#define SA_ENEMY_H

#include "bullet.hpp"
#include "model.hpp"
#include "sa.hpp"
#include "saobject.hpp"
#include "shield.hpp"

class Enemy : public SAObject {
private:
    BulletProto m_weapon{};
    Shield m_shield;

    GLdouble m_visibleRange = 0.0;
    GLdouble m_outRange = 0.0;
    GLdouble m_shotFactor = 0.0;
    GLfloat m_healthPerc = 0.0f;
    GLint m_shieldStrength = 0;

    void ReinitCoordinates();

public:
    virtual ~Enemy() override = default;
    Enemy();
    virtual void Draw() const override;
    static void DrawCollisionIndicator();
    void Update();
    void SetModel( Model* M );
    void SetVisibleRange( const GLdouble& Range );
    void SetWeapon( const BulletProto& b );
    Bullet* GetWeapon();
    bool IsWeaponReady() const;
    void DrawRadarPosition( const Vertex& Modifier, const GLdouble& RadarScale ) const;

    virtual void ProcessCollision( SAObject* Object ) override;

};

#endif
