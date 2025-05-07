#pragma once

#include "autolerp.hpp"
#include "bullet.hpp"
#include "enemy.hpp"
#include "explosion.hpp"
#include "player.hpp"
#include "skybox.hpp"
#include "space_dust.hpp"
#include "update_context.hpp"
#include "game_action.hpp"
#include "targeting.hpp"

#include <audio/audio.hpp>
#include <renderer/texture.hpp>

#include <memory_resource>
#include <vector>


class GameScene {
    bool m_pause = true;
    Skybox m_skybox{};
    Player m_player{};
    Targeting m_targeting{};
    std::pmr::vector<Bullet> m_bullets{};
    std::pmr::vector<Explosion> m_explosions{};
    std::pmr::vector<Enemy> m_enemies{};
    SpaceDust m_spacedust{};
    Player::Input m_playerInput{};
    Texture m_plasma{};
    Audio* m_audio{};
    AutoLerp<float> m_look{ 0.0f, 1.0f, 3.0f };
    uint32_t m_score = 0;

    void retarget();

public:
    struct CreateInfo {
        Audio* audio{};
        std::array<Texture, 6> skybox{};
        Texture plasma{};
        Model* enemyModel{};
        WeaponCreateInfo enemyWeapon{};
        std::span<const csg::Callsign> enemyCallsigns{};
        Player::CreateInfo player{};
    };

    ~GameScene() noexcept = default;
    GameScene() noexcept = default;
    GameScene( const CreateInfo& ) noexcept;

    void render( RenderContext );
    void update( UpdateContext );
    void onAction( input::Action );

    std::pmr::vector<Explosion>& explosions();
    std::pmr::vector<Bullet>& projectiles();
    std::pmr::vector<Enemy>& enemies();
    Player& player();
    const Player& player() const;
    void setPause( bool );
    bool isPause() const;
    uint32_t score() const;
    std::pmr::vector<Signal> signals() const;

    std::tuple<math::mat4, math::mat4> getCameraMatrix( float aspect ) const;
    std::tuple<math::vec3, math::vec3, math::vec3> getCamera() const;
};
