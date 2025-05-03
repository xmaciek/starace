#pragma once

#include "bullet.hpp"
#include "explosion.hpp"
#include "enemy.hpp"
#include "skybox.hpp"
#include "space_dust.hpp"
#include "update_context.hpp"

#include <renderer/texture.hpp>

#include <memory_resource>
#include <vector>


class GameScene {
    Skybox m_skybox{};
    SpaceDust m_spacedust{};
    std::pmr::vector<Bullet> m_bullets{};
    std::pmr::vector<Explosion> m_explosions{};
    std::pmr::vector<Enemy> m_enemies{};

    Texture m_plasma{};

public:
    ~GameScene() noexcept = default;
    GameScene() noexcept = default;
    GameScene( const std::array<Texture, 6>& t, Texture plasma ) noexcept;

    void render( const RenderContext& );
    void update( const UpdateContext&, math::vec3 jetPos, math::vec3 jetVel );

    std::pmr::vector<Explosion>& explosions();
    std::pmr::vector<Bullet>& projectiles();
    std::pmr::vector<Enemy>& enemies();
};
