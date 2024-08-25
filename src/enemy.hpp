#pragma once

#include "bullet.hpp"
#include "saobject.hpp"
#include "model.hpp"

#include <engine/math.hpp>
#include <shared/pmr_pointer.hpp>

#include <span>
#include <vector>


class Enemy : public SAObject {
private:
    WeaponCreateInfo m_weapon{};
    Model m_model{};
    float m_shotFactor = 0.0f;
    uint16_t m_callsign = 0xFFFF;
    math::quat quat() const;

public:
    static constexpr inline uint16_t COLLIDE_ID = 'EN';
    struct CreateInfo {
        Model* model = nullptr;
        uint16_t callsign = 0xFFFF;
    };
    virtual ~Enemy() override = default;
    Enemy( const CreateInfo& );

    void shoot( std::pmr::vector<Bullet>& );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    void setWeapon( const WeaponCreateInfo& );
    Signal signal() const;

    static void renderAll( const RenderContext&, std::span<const UniquePointer<Enemy>> );
    static void updateAll( const UpdateContext&, std::span<UniquePointer<Enemy>> );
};
