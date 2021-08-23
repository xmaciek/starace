#pragma once

#include "bullet.hpp"
#include "saobject.hpp"
#include "model.hpp"
#include "thruster.hpp"

class Enemy : public SAObject {
private:
    BulletProto m_weapon{};
    Thruster m_thruster{};
    Model* m_model = nullptr;
    glm::vec3 m_screenPos{};
    float m_shotFactor = 0.0f;
    float m_healthPerc = 1.0f;
    bool m_isOnScreen = false;

public:
    virtual ~Enemy() override = default;
    Enemy( Model* );

    Bullet* weapon( void* );
    bool isWeaponReady() const;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setWeapon( const BulletProto& b );
};
