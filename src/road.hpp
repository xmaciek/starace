#ifndef StarAce_ROAD
#define StarAce_ROAD

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

    Circle* Radar = nullptr;
    Circle* speed_fan_ring = nullptr;
    Font* font_big = nullptr;
    Font* font_gui_txt = nullptr;
    Font* font_pause_txt = nullptr;
    Jet* jet = nullptr;
    Map* map = nullptr;
    Mix_Chunk* blaster = nullptr;
    Mix_Chunk* click = nullptr;
    Mix_Chunk* laser = nullptr;
    Mix_Chunk* torpedo = nullptr;
    SDL_Surface* Display = nullptr;
    SDL_Thread* thread = nullptr;

    std::string LastSelectedJetName{};
    std::vector<Bullet*> Bgarbage{};
    std::vector<Bullet*> bullet{};
    std::vector<Bullet*> enemybullet{};
    std::vector<Enemy*> Egarbage{};
    std::vector<Enemy*> enemies{};
    std::vector<MapProto> maps_container{};
    std::vector<ModelProto> jet_models_proto{};
    std::vector<ModelProto> jets_container{};

    std::mutex m_mutexBullet{};
    std::mutex m_mutexEnemyBullet{};
    std::mutex m_mutexEnemy{};
    time_t TimePassed{};

    Model preview_model{};

    BulletProto Weapons[ 4 ]{};
    Button Options{};
    Button btnChangeFiltering{};
    Button btnCustomizeReturn{};
    Button btnCustomize{};
    Button btnExit{};
    Button btnGO{};
    Button btnNextJet{};
    Button btnNextMap{};
    Button btnPrevJet{};
    Button btnPrevMap{};
    Button btnQuitMission{};
    Button btnReturnToMainMenu{};
    Button btnReturnToMissionSelection{};
    Button btnSelectMissionCancel{};
    Button btnSelectMission{};
    Button btnStartMission{};
    Button btnWeap1{};
    Button btnWeap2{};
    Button btnWeap3{};

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
    bool DynamicCamera = false;
    bool FULLSCREEN = false;
    bool Running = false;
    bool WaitForEnd = false;
    bool background_effect_equation = false;
    bool cyber_ring_rotation_direction[ 3 ]{};
    bool doUpdate = false;
    bool play_sound = false;
    char HUDMESSAGE[ 48 ]{};

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

#endif
