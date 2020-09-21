#pragma once

#include "sa.hpp"

#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "font.hpp"
#include "jet.hpp"
#include "map.hpp"
#include "render_context.hpp"
#include "texture.hpp"
#include "update_context.hpp"

#include <mutex>
#include <thread>

constexpr static int TAB = 9;
constexpr static int ESC = 27;

class Road {
public:
    Road();
    ~Road();

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

    Circle* m_radar = nullptr;
    Circle* m_speedFanRing = nullptr;
    Font* m_fontBig = nullptr;
    Font* m_fontGuiTxt = nullptr;
    Font* m_fontPauseTxt = nullptr;
    Jet* m_jet = nullptr;
    Map* m_map = nullptr;
    Mix_Chunk* m_blaster = nullptr;
    Mix_Chunk* m_click = nullptr;
    Mix_Chunk* m_laser = nullptr;
    Mix_Chunk* m_torpedo = nullptr;
    SDL_Surface* m_display = nullptr;
    std::thread m_thread{};

    std::string m_lastSelectedJetName{};
    std::vector<Bullet*> m_bulletGarbage{};
    std::vector<Bullet*> m_bullets{};
    std::vector<Bullet*> m_enemyBullets{};
    std::vector<Enemy*> m_enemyGarbage{};
    std::vector<Enemy*> m_enemies{};
    std::vector<MapProto> m_mapsContainer{};
    std::vector<ModelProto> m_jetModels{};
    std::vector<ModelProto> m_jetsContainer{};

    std::mutex m_mutexBullet{};
    std::mutex m_mutexEnemyBullet{};
    std::mutex m_mutexEnemy{};
    time_t m_timePassed{};

    Model m_previewModel{};

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

    double m_viewportHeight = 540.0;
    double m_viewportWidth = 960;

    double m_alphaValue = 0.0;
    double m_calculatedFPS = 0.0;
    double m_rotation = 0.0;
    double m_modelRotation = 0.0;
    double m_speedAnim = 0.0;
    double m_tempFPS = 0.0;
    float m_angle = 55.0f;
    float m_cyberRingRotation[ 3 ]{};
    float m_hudColor4fv[ 3 ][ 4 ]{};
    float m_lightAmbient[ 4 ]{};
    float m_lightDiffuse[ 4 ]{};
    float m_lightPosition[ 4 ]{};
    float m_cyberRingColor[ 3 ][ 4 ]{};
    int32_t m_currentResolution = 0;
    int32_t m_maxDimention = 0;
    int32_t m_minDimention = 0;
    uint32_t m_buttonTexture = 0;
    uint32_t m_fps = 0;
    uint32_t m_framesDone = 0;
    uint32_t m_hudColor = 0.0;
    uint32_t m_hudTex = 0;
    uint32_t m_resolutions[ 4 ][ 2 ]{};
    uint32_t m_shotsDone = 0;
    uint32_t m_currentJet = 0;
    uint32_t m_currentMap = 0;
    uint32_t m_cyberRingTexture[ 3 ]{};
    uint32_t m_menuBackground = 0;
    uint32_t m_menuBackgroundOverlay = 0;
    uint32_t m_starfieldTexture = 0;
    Uint32 m_nextFrame = 0;
    Uint32 m_nextTick = 0;
    Uint32 m_timeS = 0;

    Screen m_currentScreen = Screen::eGame;
    uint8_t m_weap1 = 0;
    uint8_t m_weap2 = 0;
    uint8_t m_weap3 = 0;
    bool m_isDynamicCamera = true;
    bool m_isFullscreen = false;
    bool m_isRunning = true;
    bool m_waitForEnd = true;
    bool m_backgroundEffectEquation = false;
    bool m_cyberRingRotationDirection[ 3 ]{};
    bool m_doUpdate = false;
    bool m_isSoundEnabled = false;

    bool initNewSurface( int32_t w, int32_t h, int32_t d, bool f );
    bool onInit();
    double viewportHeight() const;
    double viewportWidth() const;
    static void drawAxis();
    static void drawHUDLine( double x1, double y1, double x2, double y2, double t );
    static void drawHUDPiece( double x, double y, double rotAngleZ );
    static void drawHudRect( double x, double y, double w, double h );
    static void drawLine( double x, double y );
    // purposefully copy argument
    void addBullet( uint32_t wID );
    void changeScreen( Screen );
    void clearMapData();
    void createMapData( const MapProto&, const ModelProto& );
    void gameKeyboardBriefingPressed( SDLKey );
    void gameKeyboardPausedPressed( SDLKey );
    void gameKeyboardPausedUnpressed( SDLKey );
    void gameKeyboardPressed( SDLKey );
    void gameKeyboardUnpressed( SDLKey );
    void goFullscreen( bool );
    void initRoadAdditionsGL();
    void loadConfig();
    void loadJetProto();
    void loadMapProto();
    void onCleanup();
    void onEvent( const SDL_Event& );
    void onKeyDown( SDLKey sym, SDLMod mod, Uint16 unicode );
    void onKeyUp( SDLKey sym, SDLMod mod, Uint16 unicode );
    void onMouseClickLeft( int32_t x, int32_t y );
    void onRender();
    void onResize( int32_t w, int32_t h );
    void onUpdate();
    void pause();
    void playSound( Mix_Chunk* ) const;
    void render3D( RenderContext );
    void renderClouds( RenderContext ) const;
    void renderCrosshair( RenderContext );
    void renderCyberRings( RenderContext );
    void renderCyberRingsMini( RenderContext );
    void renderDeadScreen( RenderContext );
    void renderGameScreen( RenderContext );
    void renderGameScreenBriefing( RenderContext );
    void renderGameScreenPaused( RenderContext );
    void renderHUD( RenderContext );
    void renderHUDBar( uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t current, uint32_t max );
    void renderMainMenu( RenderContext );
    void renderMissionSelectionScreen( RenderContext );
    void renderPauseText( RenderContext );
    void renderScreenCustomize( RenderContext );
    void renderWinScreen( RenderContext );
    void retarget();
    void saveConfig();
    void setCamera();
    void setOrtho() const;
    void setPerspective( double ) const;
    void setViewportSize( double, double );
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
