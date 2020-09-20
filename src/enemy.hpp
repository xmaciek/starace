#pragma once

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
    GLdouble m_outRange = 1.0;
    GLdouble m_shotFactor = 0.0;
    GLfloat m_healthPerc = 1.0f;
    GLint m_shieldStrength = 0;

    void reinitCoordinates();

public:
    virtual ~Enemy() override = default;
    Enemy();

    Bullet* weapon();
    bool isWeaponReady() const;
    static void drawCollisionIndicator();
    virtual void draw() const override;
    virtual void processCollision( SAObject* ) override;
    void drawRadarPosition( const Vertex& modifier, GLdouble scale ) const;
    void setModel( Model* );
    void setVisibleRange( GLdouble );
    void setWeapon( const BulletProto& b );
    void update();

};
