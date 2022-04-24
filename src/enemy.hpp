#pragma once

#include "bullet.hpp"
#include "saobject.hpp"
#include "model.hpp"
#include "thruster.hpp"

#include <engine/math.hpp>
#include <shared/pmr_pointer.hpp>

#include <memory_resource>

class Enemy : public SAObject {
private:
    BulletProto m_weapon{};
    Thruster m_thruster{};
    Model* m_model = nullptr;
    float m_shotFactor = 0.0f;

public:
    virtual ~Enemy() override = default;
    Enemy( Model* );

    UniquePointer<Bullet> weapon( std::pmr::memory_resource* );
    bool isWeaponReady() const;
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setWeapon( const BulletProto& b );
};
