#pragma once

#include "bullet.hpp"
#include "saobject.hpp"
#include "shield.hpp"

class Enemy : public SAObject {
private:
    BulletProto m_weapon{};
    Shield m_shield;
    glm::vec3 m_screenPos{};
    float m_shotFactor = 0.0f;
    float m_healthPerc = 1.0f;
    bool m_isOnScreen = false;
    void reinitCoordinates();

public:
    virtual ~Enemy() override = default;
    Enemy();

    Bullet* weapon( void* );
    bool isWeaponReady() const;
    static void drawCollisionIndicator();
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void drawRadarPosition( const glm::vec3& modifier, float scale ) const;
    void setWeapon( const BulletProto& b );
};
