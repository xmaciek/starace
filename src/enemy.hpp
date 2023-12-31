#pragma once

#include "bullet.hpp"
#include "saobject.hpp"
#include "model.hpp"
#include "thruster.hpp"

#include <engine/math.hpp>
#include <shared/pmr_pointer.hpp>

#include <span>
#include <vector>

class Enemy : public SAObject {
private:
    WeaponCreateInfo m_weapon{};
    Thruster m_thruster{};
    Model m_model{};
    float m_shotFactor = 0.0f;

public:
    static constexpr inline uint16_t COLLIDE_ID = 'EN';
    virtual ~Enemy() override = default;
    Enemy( Model* );

    void shoot( std::pmr::vector<Bullet>& );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setWeapon( const WeaponCreateInfo& );

    static void renderAll( const RenderContext&, std::span<const UniquePointer<Enemy>> );
    static void updateAll( const UpdateContext&, std::span<UniquePointer<Enemy>> );
};
