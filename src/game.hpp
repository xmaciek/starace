#pragma once

#include "action_state_tracker.hpp"
#include "autolerp.hpp"
#include "bullet.hpp"
#include "enemy.hpp"
#include "game_options.hpp"
#include "game_scene.hpp"
#include "jet.hpp"
#include "map_create_info.hpp"
#include "model_proto.hpp"
#include "space_dust.hpp"
#include "targeting.hpp"
#include "texture.hpp"
#include "ui_glow.hpp"
#include "ui_rings.hpp"

#include <config/config.hpp>
#include <engine/engine.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <extra/csg.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <shared/pool.hpp>
#include <ui/atlas.hpp>
#include <ui/data_model.hpp>
#include <ui/font.hpp>
#include <ui/remapper.hpp>
#include <ui/screen.hpp>
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
        eInit,
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

    ui::Remapper m_uiRemapper{};
    ui::Font m_uiAtlas{};
    ui::Font m_inputXbox{};
    ui::Font m_fontSmall{};
    ui::Font m_fontMedium{};
    ui::Font m_fontLarge{};

    uint32_t m_currentMission = 0;
    uint32_t m_currentJet = 0;
    uint32_t m_weapon1 = 0;
    uint32_t m_weapon2 = 1;
    uint32_t m_score = 0;
    Model m_enemyModel{};
    Jet m_jet{};
    GameScene m_gameScene{};

    Audio::Slot m_blaster{};
    Audio::Slot m_click{};
    Audio::Slot m_torpedo{};

    std::pmr::vector<Bullet> m_bullets{};
    Pool<Enemy, 100> m_poolEnemies{};
    std::pmr::vector<UniquePointer<Enemy>> m_enemies{};
    std::pmr::vector<MapCreateInfo> m_mapsContainer{};
    std::pmr::vector<ModelProto> m_jetsContainer{};
    std::pmr::map<std::filesystem::path, Mesh> m_meshes{};

    std::pmr::vector<WeaponCreateInfo> m_weapons{};
    WeaponCreateInfo m_enemyWeapon{};
    UIRings m_uiRings{};


    Texture m_cyberRingTexture{};
    Texture m_plasma{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};
    std::pmr::vector<csg::Callsign> m_callsigns{};

    Glow m_glow{};
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
    AutoLerp<float> m_lookAtTarget{ 0.0f, 1.0f, 3.0f };

    Jet::Input m_jetInput{};
    std::atomic<Screen> m_currentScreen = Screen::eInit;

    ui::GenericDataModel m_dataMissionSelect{};

    OptionsGFX m_optionsGFX{};
    OptionsCustomize m_optionsCustomize{};
    GameplayUIData m_gameplayUIData{};

    ui::Var<std::pmr::u32string> m_uiMissionResult{ U"BUG ME" };
    ui::Var<std::pmr::u32string> m_uiMissionScore{ U"BUG ME" };

    ActionStateTracker m_actionStateTracker{};

    FixedMap<Hash::value_type, ui::DataModel*, 64> m_gameUiDataModels{};
    FixedMap<Hash::value_type, std::function<void()>, 64> m_gameCallbacks{};
    FixedMap<Hash::value_type, std::pmr::u32string, 64> m_localizationMap{};

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void changeScreen( Screen, Audio::Slot sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void loadMapProto();
    void pause();

    ui::Screen* currentScreen();

    std::tuple<math::vec3, math::vec3, math::vec3> getCamera() const;
    std::tuple<math::mat4, math::mat4> getCameraMatrix() const;

    void setupLocalization();
    void setupUI();

    // purposefully copy argument
    void render3D( RenderContext );
    void renderBackground( ui::RenderContext ) const;
    void renderGameScreen( RenderContext );
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
