#pragma once

#include "action_state_tracker.hpp"
#include "bullet.hpp"
#include "enemy.hpp"
#include "explosion.hpp"
#include "game_options.hpp"
#include "hud.hpp"
#include "jet.hpp"
#include "map_create_info.hpp"
#include "model_proto.hpp"
#include "skybox.hpp"
#include "space_dust.hpp"
#include "targeting.hpp"
#include "texture.hpp"
#include "ui_glow.hpp"
#include "ui_rings.hpp"
#include "ui_screen.hpp"

#include <config/config.hpp>
#include <engine/engine.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <shared/pool.hpp>
#include <ui/data_model.hpp>
#include <ui/font.hpp>
#include <ui/linear_atlas.hpp>
#include <ui/var.hpp>

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
    enum class Screen : uint32_t {
        eGame,
        eGameBriefing,
        eGamePaused,
        eGameResult,
        eMissionSelection,
        eDead,
        eWin,
        eCustomize,
        eMainMenu,
        eSettings,
        eSettingsDisplay,
        max,
    };

    UniquePointer<ui::Font> m_fontSmall{};
    UniquePointer<ui::Font> m_fontMedium{};
    UniquePointer<ui::Font> m_fontLarge{};
    uint32_t m_currentMission = 0;
    uint32_t m_currentJet = 0;
    uint32_t m_weapon1 = 0;
    uint32_t m_weapon2 = 1;
    Model m_enemyModel{};
    Jet m_jet{};
    Skybox m_skybox{};
    ui::LinearAtlas m_atlasUi{};

    Audio::Slot m_blaster{};
    Audio::Slot m_click{};
    Audio::Slot m_torpedo{};

    Pool<Bullet, 1024> m_poolBullets{};
    Pool<Enemy, 100> m_poolEnemies{};
    std::pmr::vector<UniquePointer<Bullet>> m_bullets{};
    std::pmr::vector<UniquePointer<Bullet>> m_enemyBullets{};
    std::pmr::vector<UniquePointer<Enemy>> m_enemies{};
    std::pmr::vector<Explosion> m_explosions{};
    std::pmr::vector<MapCreateInfo> m_mapsContainer{};
    std::pmr::vector<ModelProto> m_jetsContainer{};
    std::pmr::map<std::filesystem::path, Mesh> m_meshes{};

    std::pmr::vector<WeaponCreateInfo> m_weapons{};
    WeaponCreateInfo m_enemyWeapon{};

    HudData m_hudData{};
    Hud m_hud{};
    UIRings m_uiRings{};


    Texture m_cyberRingTexture[ 3 ]{};
    Texture m_plasma{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};

    Glow m_glow{};
    SpaceDust m_dustGame{};
    SpaceDust m_dustUi{};
    ui::Screen m_screenCustomize{};
    ui::Screen m_screenGameplay{};
    ui::Screen m_screenMissionResult{};
    ui::Screen m_screenMissionSelect{};
    ui::Screen m_screenPause{};
    ui::Screen m_screenSettingsDisplay{};
    ui::Screen m_screenSettings{};
    ui::Screen m_screenTitle{};

    Targeting m_targeting{};

    Jet::Input m_jetInput{};
    Screen m_currentScreen = Screen::eGame;

    ui::GenericDataModel m_dataMissionSelect{};

    OptionsGFX m_optionsGFX{};
    OptionsCustomize m_optionsCustomize{};

    ui::Var<std::pmr::u32string> m_uiMissionResult{ "$var:missionResult", U"BUG ME" };
    ui::Var<std::pmr::u32string> m_uiMissionScore{ "$var:missionScore", U"BUG ME" };
    ui::Var<float> m_uiPlayerHP{ "$var:playerHP", 0.0f };

    LocTable m_localizationMap{};

    ActionStateTracker m_actionStateTracker{};

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void playSounds( std::span<UniquePointer<Bullet>> );
    void changeScreen( Screen, Audio::Slot sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void loadMapProto();
    void pause();

    ui::Screen* currentScreen();

    std::tuple<math::vec3, math::vec3, math::vec3> getCamera() const;
    std::tuple<math::mat4, math::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderBackground( ui::RenderContext ) const;
    void renderGameScreen( RenderContext, ui::RenderContext );
    void renderMenuScreen( RenderContext, ui::RenderContext ) const;

    void retarget();
    void setCamera();
    void unpause();
    void updateGame( const UpdateContext& );

    void onAction( Action );
    virtual void onActuator( Actuator ) override;
    virtual void onInit() override;
    virtual void onExit() override;
    virtual void onRender( RenderContext ) override;
    virtual void onUpdate( const UpdateContext& ) override;
    virtual void onResize( uint32_t, uint32_t ) override;
    virtual void onMouseEvent( const MouseEvent& ) override;
};
