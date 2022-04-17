#pragma once

#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "explosion.hpp"
#include "font.hpp"
#include "hud.hpp"
#include "jet.hpp"
#include "linear_atlas.hpp"
#include "map_create_info.hpp"
#include "model_proto.hpp"
#include "screen_pause.hpp"
#include "screen_win_loose.hpp"
#include "skybox.hpp"
#include "space_dust.hpp"
#include "targeting.hpp"
#include "texture.hpp"
#include "ui_rings.hpp"
#include "ui_screen.hpp"
#include "ui_glow.hpp"
#include "ui_data_model.hpp"

#include <engine/engine.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>
#include <shared/pool.hpp>

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
        eSettings,
        eMainMenu,
    };

    Font* m_fontSmall = nullptr;
    Font* m_fontMedium = nullptr;
    Font* m_fontLarge = nullptr;
    Model* m_enemyModel = nullptr;
    uint32_t m_currentMission = 0;
    uint32_t m_currentJet = 0;
    uint32_t m_weapon1 = 0;
    uint32_t m_weapon2 = 0;
    uint32_t m_weapon3 = 0;
    Jet m_jet{};
    Skybox m_skybox{};
    LinearAtlas m_atlasUi{};

    Audio::Slot m_blaster{};
    Audio::Slot m_click{};
    Audio::Slot m_laser{};
    Audio::Slot m_torpedo{};

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

    Texture m_cyberRingTexture[ 3 ]{};
    Texture m_plasma{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};

    Glow m_glow{};
    SpaceDust m_spaceDust{};
    ui::Screen m_screenTitle{};
    ui::Screen m_screenCustomize{};
    ui::Screen m_screenSettings{};
    ui::Screen m_screenMissionSelect{};
    ScreenPause m_screenPause{};
    ScreenWinLoose m_screenWin{};
    ScreenWinLoose m_screenLoose{};

    Targeting m_targeting{};

    Jet::Input m_jetInput{};
    Screen m_currentScreen = Screen::eGame;

    ui::GenericDataModel m_dataMissionSelect{};
    ui::GenericDataModel m_dataModelVSync{};
    ui::GenericDataModel m_dataJet{};
    ui::GenericDataModel m_dataWeaponPrimary{};
    ui::GenericDataModel m_dataWeaponSecondary{};
    ui::StringListModel m_dataModelResolution{};

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void addBullet( uint32_t wID );
    void changeScreen( Screen, Audio::Slot sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void loadMapProto();
    void pause();

    std::tuple<math::mat4, math::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderBackground( RenderContext ) const;
    void renderGameScreen( RenderContext );
    void renderMenuScreen( RenderContext ) const;
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
