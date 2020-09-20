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

    GLint OnExecute();

private:
    static const GLubyte SA_GAMESCREEN = 0;
    static const GLubyte SA_GAMESCREEN_BRIEFING = 1;
    static const GLubyte SA_GAMESCREEN_PAUSED = 2;
    static const GLubyte SA_MISSIONSELECTION = 3;
    static const GLubyte SA_DEADSCREEN = 4;
    static const GLubyte SA_WINSCREEN = 5;
    static const GLubyte SA_CUSTOMIZE = 120;
    static const GLubyte SA_MAINMENU = 100;
    static const GLubyte SA_OPTIONS = 110;
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

    std::string LastSelectedJetName{};
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
    time_t TimePassed{};

    Model preview_model{};

    BulletProto Weapons[ 4 ]{};
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

    GLdouble CalculatedFPS = 0.0;
    GLdouble Rotation = 0.0;
    GLdouble cyber_ring_rotation[ 3 ]{};
    GLdouble model_rotation = 0.0;
    GLdouble speed_anim = 0.0;
    GLdouble tempFPS = 0.0;
    GLfloat HUD_Color_4fv[ 3 ][ 4 ]{};
    GLfloat LightAmbient[ 4 ]{};
    GLfloat LightDiffuse[ 4 ]{};
    GLfloat LightPosition[ 4 ]{};
    GLfloat alpha_value = 0.0f;
    GLfloat cyber_ring_color[ 3 ][ 4 ]{};
    GLint SCREEN_DEPTH = 0;
    GLint SCREEN_HEIGHT = 0;
    GLint SCREEN_WIDTH = 0;
    GLint angle = 0;
    GLint current_filtering = 0;
    GLint current_resolution = 0;
    GLint max_dimention = 0;
    GLint min_dimention = 0;
    GLuint ButtonTexture = 0;
    GLuint FPS = 0;
    GLuint FramesDone = 0;
    GLuint HUD_Color = 0.0;
    GLuint HUDtex = 0;
    GLuint RESOLUTIONS[ 4 ][ 2 ]{};
    GLuint ShotsDone = 0;
    GLuint current_jet = 0;
    GLuint current_map = 0;
    GLuint cyber_ring_texture[ 3 ]{};
    GLuint menu_background = 0;
    GLuint menu_background_overlay = 0;
    GLuint starfield_texture = 0;
    Uint32 nextFrame = 0;
    Uint32 nextTick = 0;
    Uint32 timeS = 0;

    GLubyte SCREEN = 0;
    GLubyte Weap1 = 0;
    GLubyte Weap2 = 0;
    GLubyte Weap3 = 0;
    bool m_isDynamicCamera = true;
    bool m_isFullscreen = false;
    bool m_isRunning = true;
    bool m_waitForEnd = true;
    bool m_backgroundEffectEquation = false;
    bool m_cyberRingRotationDirection[ 3 ]{};
    bool m_doUpdate = false;
    bool m_isSoundEnabled = false;

    bool InitNewSurface( GLint W, GLint H, GLint D, bool F );
    bool OnInit();
    static Uint32 Delay();
    static int OnUpdateStatic( void* param );
    static void DrawAxis();
    static void DrawHUDLine( GLdouble X1, GLdouble Y1, GLdouble X2, GLdouble Y2, GLdouble T );
    static void DrawHUDPiece( GLdouble X, GLdouble Y, GLdouble RotAngleZ );
    static void DrawHudRect( GLdouble X, GLdouble Y, GLdouble W, GLdouble H );
    static void DrawLine( GLdouble X, GLdouble Y );
    void AddBullet( GLuint wID );
    void ChangeScreen( GLubyte SCR );
    void ClearMapData();
    void CreateMapData( const MapProto& map_data, const ModelProto& model_data );
    void DeadScreen();
    void DeadScreenUpdate();
    void DrawBullets();
    void DrawClouds() const;
    void DrawCrosshair();
    void DrawCyberRings();
    void DrawCyberRingsMini();
    void DrawHUDBar( const GLuint& X, const GLuint& Y, const GLuint& W, const GLuint& H, const GLuint& Current, const GLuint& Max );
    void DrawMainMenu();
    void DrawPauseText();
    void GameKeyboardBriefingPressed( SDLKey sym );
    void GameKeyboardPausedPressed( SDLKey sym );
    void GameKeyboardPausedUnpressed( SDLKey sym );
    void GameKeyboardPressed( SDLKey sym );
    void GameKeyboardUnpressed( SDLKey sym );
    void GameScreen();
    void GameScreenBriefing();
    void GameScreenBriefingUpdate();
    void GameScreenPaused();
    void GameUpdate();
    void GameUpdatePaused();
    void GoFullscreen( bool& b );
    void InitRoadAdditionsGL();
    void LoadConfig();
    void LoadJetProto();
    void LoadMapProto();
    void MissionSelectionScreen();
    void MissionSelectionUpdate();
    void OnCleanup();
    void OnEvent( SDL_Event& Event );
    void OnKeyDown( SDLKey sym, SDLMod mod, Uint16 unicode );
    void OnKeyUp( SDLKey sym, SDLMod mod, Uint16 unicode );
    void OnMouseClickLeft( GLint X, GLint Y );
    void OnRender();
    void OnResize( GLint w, GLint h );
    void OnUpdate();
    void Pause();
    void PlaySound( Mix_Chunk* sound ) const;
    void Render3D();
    void RenderHUD();
    void Retarget();
    void SaveConfig();
    void ScreenCustomize();
    void SetOrtho() const;
    void SetPerspective( GLdouble Angle ) const;
    void Unpause();
    void UpdateClouds();
    void UpdateCustomize();
    void UpdateCyberRings();
    void UpdateMainMenu();
    void WinScreen();
    void WinUpdate();
    void setCamera();
};
