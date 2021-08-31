#pragma once

#include "async_io.hpp"
#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "font.hpp"
#include "fps_meter.hpp"
#include "jet.hpp"
#include "map.hpp"
#include "model_proto.hpp"
#include "pool.hpp"
#include "render_context.hpp"
#include "texture.hpp"
#include "update_context.hpp"
#include "targeting.hpp"

#include <audio/audio.hpp>
#include <renderer/texture.hpp>
#include <renderer/renderer.hpp>

#include <glm/vec4.hpp>
#include <SDL2/SDL.h>

#include <mutex>
#include <thread>
#include <tuple>

struct Mix_Chunk;

class Game {
public:
    Game();
    ~Game();

    int32_t run();

private:
    enum struct Screen {
        eGame,
        eGameBriefing,
        eGamePaused,
        eMissionSelection,
        eDead,
        eWin,
        eCustomize,
        eMainMenu,
    };

    std::unique_ptr<asyncio::Service> m_io{};
    audio::Engine* m_audio = nullptr;
    Font* m_fontBig = nullptr;
    Font* m_fontGuiTxt = nullptr;
    Font* m_fontPauseTxt = nullptr;
    Jet* m_jet = nullptr;
    Map* m_map = nullptr;
    Model* m_previewModel = nullptr;
    Renderer* m_renderer = nullptr;
    SDL_Window* m_display = nullptr;
    std::thread m_thread{};

    Model* m_enemyModel = nullptr;

    audio::Chunk m_blaster{};
    audio::Chunk m_click{};
    audio::Chunk m_laser{};
    audio::Chunk m_torpedo{};

    std::vector<SDL_Event> m_events{};
    std::mutex m_eventsBottleneck{};

    std::string m_lastSelectedJetName{};
    Pool<Bullet, 1024> m_poolBullets{};
    Pool<Enemy, 100> m_poolEnemies{};
    std::vector<Bullet*> m_bullets{};
    std::vector<Bullet*> m_enemyBullets{};
    std::vector<Enemy*> m_enemies{};
    std::vector<MapProto> m_mapsContainer{};
    std::vector<ModelProto> m_jetsContainer{};

    std::mutex m_mutexJet{};

    FPSMeter m_fpsMeter{};

    BulletProto m_weapons[ 4 ]{};
    Button m_btnChangeFiltering{};
    Button m_btnCustomizeReturn{};
    Button m_btnCustomize{};
    Button m_btnExit{};
    Button m_btnGO{};
    Button m_btnNextJet{};
    Button m_btnNextMap{};
    Button m_btnPrevJet{};
    Button m_btnPrevMap{};
    Button m_btnQuitMission{};
    Button m_btnReturnToMainMenu{};
    Button m_btnReturnToMissionSelection{};
    Button m_btnSelectMissionCancel{};
    Button m_btnSelectMission{};
    Button m_btnStartMission{};
    Button m_btnWeap1{};
    Button m_btnWeap2{};
    Button m_btnWeap3{};

    Label m_lblPaused{};

    Texture m_buttonTexture{};
    Texture m_cyberRingTexture[ 3 ]{};
    Texture m_hudTex{};
    Texture m_menuBackgroundOverlay{};
    Texture m_menuBackground{};
    Texture m_starfieldTexture{};

    Targeting m_targeting{};

    glm::vec4 m_currentHudColor = color::winScreen;
    double m_rotation = 0.0;
    double m_modelRotation = 0.0;
    double m_speedAnim = 0.0;
    float m_alphaValue = 1.0f;
    float m_alphaN = 0.0f;
    float m_angle = 55.0f;
    float m_cyberRingRotation[ 3 ]{};
    int32_t m_currentResolution = 0;
    uint32_t m_maxDimention = 0;
    uint32_t m_minDimention = 0;
    uint32_t m_viewportHeight = 540.0;
    uint32_t m_viewportWidth = 960;
    float m_viewportAspect = 0.0f;
    uint32_t m_currentJet = 0;
    uint32_t m_currentMap = 0;
    uint32_t m_resolutions[ 4 ][ 2 ]{};
    uint32_t m_shotsDone = 0;

    Screen m_currentScreen = Screen::eGame;
    uint8_t m_weap1 = 0;
    uint8_t m_weap2 = 0;
    uint8_t m_weap3 = 0;
    bool m_isDynamicCamera = true;
    bool m_isFullscreen = false;
    bool m_isRunning = true;
    bool m_waitForEnd = true;
    bool m_cyberRingRotationDirection[ 3 ]{};
    bool m_doUpdate = false;
    bool m_isSoundEnabled = false;

    bool onInit();
    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void addBullet( uint32_t wID );
    void changeScreen( Screen );
    void clearMapData();
    void createMapData( const MapProto&, const ModelProto& );
    void gameKeyboardBriefingPressed( SDL_Scancode );
    void gameKeyboardPausedPressed( SDL_Scancode );
    void gameKeyboardPausedUnpressed( SDL_Scancode );
    void gameKeyboardPressed( SDL_Scancode );
    void gameKeyboardUnpressed( SDL_Scancode );
    void goFullscreen( bool );
    void initRoadAdditions();
    void loadConfig();
    void loadJetProto();
    void loadMapProto();
    void loopGame();
    void onCleanup();
    void onEvent( const SDL_Event& );
    void onKeyDown( const SDL_Keysym& );
    void onKeyUp( const SDL_Keysym& );
    void onMouseClickLeft( int32_t x, int32_t y );
    void onMouseMove( const SDL_MouseMotionEvent& );
    void onRender();
    void onResize( uint32_t w, uint32_t h );
    void onUpdate( const UpdateContext& );
    void pause();
    void playSound( Mix_Chunk* ) const;
    void reloadPreviewModel();

    std::tuple<glm::mat4, glm::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderClouds( RenderContext ) const;
    void renderCrosshair( RenderContext );
    void renderCyberRings( RenderContext );
    void renderDeadScreen( RenderContext );
    void renderGameScreen( RenderContext );
    void renderGameScreenBriefing( RenderContext );
    void renderGameScreenPaused( RenderContext );
    void renderHUD( RenderContext );
    void renderHudTex( RenderContext, const glm::vec4& );
    void renderHUDBar( RenderContext, const glm::vec4& xywh, float ratio );
    void renderMainMenu( RenderContext );
    void renderMissionSelectionScreen( RenderContext );
    void renderPauseText( RenderContext );
    void renderScreenCustomize( RenderContext );
    void renderWinScreen( RenderContext );

    void retarget();
    void saveConfig();
    void setCamera();
    void setViewportSize( uint32_t, uint32_t );
    void unpause();
    void updateClouds( const UpdateContext& );
    void updateCustomize( const UpdateContext& );
    void updateCyberRings( const UpdateContext& );
    void updateDeadScreen( const UpdateContext& );
    void updateGame( const UpdateContext& );
    void updateGamePaused( const UpdateContext& );
    void updateGameScreenBriefing( const UpdateContext& );
    void updateMainMenu( const UpdateContext& );
    void updateMissionSelection( const UpdateContext& );
    void updateWin( const UpdateContext& );
};
