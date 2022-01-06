#pragma once

#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "font.hpp"
#include "hud.hpp"
#include "jet.hpp"
#include "map.hpp"
#include "model_proto.hpp"
#include "texture.hpp"
#include "targeting.hpp"
#include "screen_customize.hpp"
#include "screen_mission_select.hpp"
#include "screen_pause.hpp"
#include "screen_title.hpp"
#include "screen_win_loose.hpp"
#include "ui_rings.hpp"

#include <engine/engine.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>
#include <shared/pool.hpp>
#include <shared/rotary_index.hpp>

#include <glm/vec4.hpp>
#include <SDL2/SDL.h>

#include <mutex>
#include <thread>
#include <tuple>
#include <map>

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
    Jet* m_jet = nullptr;
    Map* m_map = nullptr;
    Model* m_enemyModel = nullptr;

    Audio::Chunk m_blaster{};
    Audio::Chunk m_click{};
    Audio::Chunk m_laser{};
    Audio::Chunk m_torpedo{};

    std::string m_lastSelectedJetName{};
    Pool<Bullet, 1024> m_poolBullets{};
    Pool<Enemy, 100> m_poolEnemies{};
    std::vector<Bullet*> m_bullets{};
    std::vector<Bullet*> m_enemyBullets{};
    std::vector<Enemy*> m_enemies{};
    std::vector<MapCreateInfo> m_mapsContainer{};
    std::vector<ModelProto> m_jetsContainer{};

    HudData m_hudData{};
    Hud m_hud{};
    UIRings m_uiRings{};

    BulletProto m_weapons[ 4 ]{};

    Texture m_buttonTexture{};
    Texture m_cyberRingTexture[ 3 ]{};
    Texture m_menuBackgroundOverlay{};
    Texture m_menuBackground{};
    Texture m_starfieldTexture{};
    std::pmr::map<std::filesystem::path, Texture> m_textures{};

    ScreenTitle m_screenTitle{};
    ScreenCustomize m_screenCustomize{};
    ScreenMissionSelect m_screenMissionSelect{};
    ScreenPause m_screenPause{};
    ScreenWinLoose m_screenWin{};
    ScreenWinLoose m_screenLoose{};

    Targeting m_targeting{};

    Jet::Input m_jetInput{};
    float m_maxDimention = 0.0f;
    float m_alphaValue = 1.0f;
    float m_alphaN = 0.0f;
    uint32_t m_shotsDone = 0;

    Screen m_currentScreen = Screen::eGame;

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void addBullet( uint32_t wID );
    void changeScreen( Screen, Audio::Chunk sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void loadJetProto();
    void loadMapProto();
    void onResize( uint32_t w, uint32_t h );
    void pause();

    std::tuple<glm::mat4, glm::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderClouds( RenderContext ) const;
    void renderCrosshair( RenderContext );
    void renderDeadScreen( RenderContext );
    void renderGameScreen( RenderContext );
    void renderHUD( RenderContext );
    void renderMainMenu( RenderContext );
    void renderPauseText( RenderContext );
    void renderScreenCustomize( RenderContext );
    void renderWinScreen( RenderContext );

    void preloadData();
    void retarget();
    void setCamera();
    void unpause();
    void updateClouds( const UpdateContext& );
    void updateGame( const UpdateContext& );

    virtual void onAction( Action ) override;
    virtual void onInit() override;
    virtual void onExit() override;
    virtual void onRender( RenderContext ) override;
    virtual void onUpdate( const UpdateContext& ) override;
    virtual void onEvent( const SDL_Event& ) override;
    virtual void onMouseEvent( const MouseEvent& ) override;
};
