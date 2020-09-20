#pragma once

#include "sa.hpp"

#include "bullet.hpp"
#include "button.hpp"
#include "enemy.hpp"
#include "font.hpp"
#include "jet.hpp"
#include "map.hpp"
#include "texture.hpp"

#include <mutex>

constexpr static int TAB = 9;
constexpr static int ESC = 27;

class Road {
public:
    Road();
    ~Road();

    GLint run();

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

    static const Uint32 Time_Interval;

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
    SDL_Thread* m_thread = nullptr;

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

    GLdouble m_calculatedFPS = 0.0;
    GLdouble m_rotation = 0.0;
    GLdouble m_cyberRingRotation[ 3 ]{};
    GLdouble m_modelRotation = 0.0;
    GLdouble m_speedAnim = 0.0;
    GLdouble m_tempFPS = 0.0;
    GLfloat m_hudColor4fv[ 3 ][ 4 ]{};
    GLfloat m_lightAmbient[ 4 ]{};
    GLfloat m_lightDiffuse[ 4 ]{};
    GLfloat m_lightPosition[ 4 ]{};
    GLfloat m_alphaValue = 0.0f;
    GLfloat m_cyberRingColor[ 3 ][ 4 ]{};
    GLint m_angle = 55.0;
    GLint m_currentResolution = 0;
    GLint m_maxDimention = 0;
    GLint m_minDimention = 0;
    GLuint m_buttonTexture = 0;
    GLuint m_fps = 0;
    GLuint m_framesDone = 0;
    GLuint m_hudColor = 0.0;
    GLuint m_hudTex = 0;
    GLuint m_resolutions[ 4 ][ 2 ]{};
    GLuint m_shotsDone = 0;
    GLuint m_currentJet = 0;
    GLuint m_currentMap = 0;
    GLuint m_cyberRingTexture[ 3 ]{};
    GLuint m_menuBackground = 0;
    GLuint m_menuBackgroundOverlay = 0;
    GLuint m_starfieldTexture = 0;
    Uint32 m_nextFrame = 0;
    Uint32 m_nextTick = 0;
    Uint32 m_timeS = 0;

    Screen m_currentScreen = Screen::eGame;
    GLubyte m_weap1 = 0;
    GLubyte m_weap2 = 0;
    GLubyte m_weap3 = 0;
    bool m_isDynamicCamera = true;
    bool m_isFullscreen = false;
    bool m_isRunning = true;
    bool m_waitForEnd = true;
    bool m_backgroundEffectEquation = false;
    bool m_cyberRingRotationDirection[ 3 ]{};
    bool m_doUpdate = false;
    bool m_isSoundEnabled = false;

    bool initNewSurface( GLint w, GLint h, GLint d, bool f );
    bool onInit();
    double viewportHeight() const;
    double viewportWidth() const;
    static Uint32 delay();
    static int onUpdateStatic( void* param );
    static void drawAxis();
    static void drawHUDLine( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, GLdouble t );
    static void drawHUDPiece( GLdouble x, GLdouble y, GLdouble rotAngleZ );
    static void drawHudRect( GLdouble x, GLdouble y, GLdouble w, GLdouble h );
    static void drawLine( GLdouble x, GLdouble y );
    void addBullet( GLuint wID );
    void changeScreen( Screen );
    void clearMapData();
    void createMapData( const MapProto&, const ModelProto& );
    void deadScreen();
    void deadScreenUpdate();
    void drawBullets();
    void drawClouds() const;
    void drawCrosshair();
    void drawCyberRings();
    void drawCyberRingsMini();
    void drawHUDBar( GLuint x, GLuint y, GLuint w, GLuint h, GLuint current, GLuint max );
    void drawMainMenu();
    void drawPauseText();
    void gameKeyboardBriefingPressed( SDLKey );
    void gameKeyboardPausedPressed( SDLKey );
    void gameKeyboardPausedUnpressed( SDLKey );
    void gameKeyboardPressed( SDLKey );
    void gameKeyboardUnpressed( SDLKey );
    void gameScreen();
    void gameScreenBriefing();
    void gameScreenBriefingUpdate();
    void gameScreenPaused();
    void gameUpdate();
    void gameUpdatePaused();
    void goFullscreen( bool );
    void initRoadAdditionsGL();
    void loadConfig();
    void loadJetProto();
    void loadMapProto();
    void missionSelectionScreen();
    void missionSelectionUpdate();
    void onCleanup();
    void onEvent( const SDL_Event& );
    void onKeyDown( SDLKey sym, SDLMod mod, Uint16 unicode );
    void onKeyUp( SDLKey sym, SDLMod mod, Uint16 unicode );
    void onMouseClickLeft( GLint x, GLint y );
    void onRender();
    void onResize( GLint w, GLint h );
    void onUpdate();
    void pause();
    void playSound( Mix_Chunk* ) const;
    void render3D();
    void renderHUD();
    void retarget();
    void saveConfig();
    void screenCustomize();
    void setCamera();
    void setOrtho() const;
    void setPerspective( GLdouble ) const;
    void setViewportSize( double, double );
    void unpause();
    void updateClouds();
    void updateCustomize();
    void updateCyberRings();
    void updateMainMenu();
    void winScreen();
    void winUpdate();
};
