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
    Weapon m_weapon{};
    Model m_model{};
    uint16_t m_callsign = 0xFFFF;
    math::quat quat() const;

public:
    static constexpr inline uint16_t COLLIDE_ID = 'EN';
    struct CreateInfo {
        WeaponCreateInfo weapon{};
        Model* model = nullptr;
        SAObject* target = nullptr;
        uint16_t callsign = 0xFFFF;
    };
    virtual ~Enemy() override = default;
    Enemy() = default;
    Enemy( const CreateInfo& );

    void shoot( std::pmr::vector<Bullet>& );
    Signal signal() const;

    static void renderAll( const RenderContext&, std::span<const Enemy> );
    static void updateAll( const UpdateContext&, std::span<Enemy> );
};
