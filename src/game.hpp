#pragma once

#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "explosion.hpp"
#include "font.hpp"
#include "hud.hpp"
#include "jet.hpp"
#include "linear_atlas.hpp"
#include "model_proto.hpp"
#include "screen_customize.hpp"
#include "screen_mission_select.hpp"
#include "screen_pause.hpp"
#include "screen_title.hpp"
#include "screen_win_loose.hpp"
#include "skybox.hpp"
#include "space_dust.hpp"
#include "targeting.hpp"
#include "texture.hpp"
#include "ui_rings.hpp"

#include <engine/engine.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>
#include <shared/pool.hpp>
#include <shared/rotary_index.hpp>

#include <SDL.h>

#include <tuple>
#include <map>
#include <vector>
#include <memory_resource>

class Game : public Engine {
public:
    Game( int argc, char** argv );
    ~Game();

private:
    enum class Screen {
        eGame,
        eGameBriefing,
        eGamePaused,
        eMissionSelection,
        eDead,
        eWin,
        eCustomize,
        eMainMenu,
    };

    Font* m_fontBig = nullptr;
    Font* m_fontGuiTxt = nullptr;
    Font* m_fontPauseTxt = nullptr;
    Model* m_enemyModel = nullptr;
    Jet m_jet{};
    Skybox m_skybox{};
    LinearAtlas m_atlasUi{};

    Audio::Chunk m_blaster{};
    Audio::Chunk m_click{};
    Audio::Chunk m_laser{};
    Audio::Chunk m_torpedo{};

    Pool<Bullet, 1024> m_poolBullets{};
    Pool<Enemy, 100> m_poolEnemies{};
    std::pmr::vector<Bullet*> m_bullets{};
    std::pmr::vector<Bullet*> m_enemyBullets{};
    std::pmr::vector<Enemy*> m_enemies{};
    std::pmr::vector<Explosion> m_explosions{};
    std::pmr::vector<MapCreateInfo> m_mapsContainer{};
    std::pmr::vector<ModelProto> m_jetsContainer{};

    HudData m_hudData{};
    Hud m_hud{};
    UIRings m_uiRings{};

    BulletProto m_weapons[ 4 ]{};

    Texture m_atlasTexture{};
    Texture m_buttonTexture{};
    Texture m_cyberRingTexture[ 3 ]{};
    Texture m_plasma{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};

    SpaceDust m_spaceDust{};
    ScreenTitle m_screenTitle{};
    ScreenCustomize m_screenCustomize{};
    ScreenMissionSelect m_screenMissionSelect{};
    ScreenPause m_screenPause{};
    ScreenWinLoose m_screenWin{};
    ScreenWinLoose m_screenLoose{};

    Targeting m_targeting{};

    Jet::Input m_jetInput{};
    Screen m_currentScreen = Screen::eGame;

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void addBullet( uint32_t wID );
    void changeScreen( Screen, Audio::Chunk sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void loadMapProto();
    void pause();

    std::tuple<math::mat4, math::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderBackground( RenderContext ) const;
    void renderGameScreen( RenderContext );
    void renderHUD( RenderContext );

    void preloadData();
    void retarget();
    void setCamera();
    void unpause();
    void updateGame( const UpdateContext& );

    virtual void onAction( Action ) override;
    virtual void onInit() override;
    virtual void onExit() override;
    virtual void onRender( RenderContext ) override;
    virtual void onUpdate( const UpdateContext& ) override;
    virtual void onResize( uint32_t, uint32_t ) override;
    virtual void onMouseEvent( const MouseEvent& ) override;
};
