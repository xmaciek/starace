#pragma once

#include "autolerp.hpp"
#include "enemy.hpp"
#include "game_options.hpp"
#include "game_scene.hpp"
#include "player.hpp"
#include "map_create_info.hpp"
#include "model_proto.hpp"
#include "space_dust.hpp"
#include "targeting.hpp"
#include "texture.hpp"

#include <config/config.hpp>
#include <engine/engine.hpp>
#include <engine/math.hpp>
#include <extra/csg.hpp>
#include <renderer/texture.hpp>
#include <shared/hash.hpp>
#include <shared/pool.hpp>
#include <ui/atlas.hpp>
#include <ui/data_model.hpp>
#include <ui/font.hpp>
#include <input/remapper.hpp>
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
    input::Remapper m_remapper{};
    ui::Font m_uiAtlas{};
    ui::Font m_inputPS4{};
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
    Player m_player{};
    GameScene m_gameScene{};
    std::pmr::list<ui::Screen> m_screens;
    ui::Screen* m_currentScreen = nullptr;

    Audio::Slot m_click{};

    Pool<Enemy, 100> m_poolEnemies{};
    std::pmr::vector<UniquePointer<Enemy>> m_enemies{};
    std::pmr::vector<MapCreateInfo> m_mapsContainer{};
    std::pmr::vector<ModelProto> m_jetsContainer{};
    std::pmr::map<std::filesystem::path, Mesh> m_meshes{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};
    std::pmr::map<std::filesystem::path, Audio::Slot> m_sounds{};

    std::pmr::vector<WeaponCreateInfo> m_weapons{};
    WeaponCreateInfo m_enemyWeapon{};

    Texture m_plasma{};
    std::pmr::vector<csg::Callsign> m_callsigns{};

    SpaceDust m_dustUi{};

    Targeting m_targeting{};
    AutoLerp<float> m_lookAtTarget{ 0.0f, 1.0f, 3.0f };

    Player::Input m_playerInput{};

    ui::GenericDataModel m_dataMissionSelect{};

    GameSettings m_gameSettings{};
    OptionsCustomize m_optionsCustomize{};
    OptionsGFX m_optionsGFX{};
    OptionsAudio m_optionsAudio{};
    OptionsGame m_optionsGame{};
    GameplayUIData m_gameplayUIData{};

    ui::Var<std::pmr::u32string> m_uiMissionResult{ U"BUG ME" };
    ui::Var<std::pmr::u32string> m_uiMissionScore{ U"BUG ME" };

    FixedMap<Hash::value_type, ui::DataModel*, 64> m_gameUiDataModels{};
    FixedMap<Hash::value_type, std::function<void()>, 64> m_gameCallbacks{};
    FixedMap<Hash::value_type, std::pmr::u32string, 64> m_localizationMap{};

    void loadDDS( const Asset& );
    void loadOBJC( const Asset& );
    void loadMAP( const Asset& );
    void loadJET( const Asset& );
    void loadWPN( const Asset& );
    void loadLANG( const Asset& );
    void loadWAV( const Asset& );
    void loadCSG( const Asset& );

    void loadSettings();
    void applyDisplay();
    void applyAudio();
    void applyGameSettings();

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void changeScreen( Hash::value_type, Audio::Slot sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void pause();

    ui::Screen* currentScreen();

    std::tuple<math::vec3, math::vec3, math::vec3> getCamera() const;
    std::tuple<math::mat4, math::mat4> getCameraMatrix() const;

    void setupUI();

    // purposefully copy argument
    void render3D( RenderContext );
    void renderBackground( ui::RenderContext ) const;
    void renderGameScreen( RenderContext );
    void renderMenuScreen( RenderContext, ui::RenderContext ) const;

    void retarget();
    void setCamera();
    void unpause();
    void updateGame( UpdateContext& );

    void onAction( input::Action );
    virtual void onActuator( input::Actuator ) override;
    virtual void onInit() override;
    virtual void onExit() override;
    virtual void onRender( Renderer* ) override;
    virtual void onUpdate( float ) override;
    virtual void onResize( uint32_t, uint32_t ) override;
    virtual void onMouseEvent( const MouseEvent& ) override;
};
