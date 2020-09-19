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

constexpr static int TAB = 9;
constexpr static int ESC = 27;

class Road {
public:
    Road();
    ~Road();

    GLint OnExecute();

private:
    bool Running = false;
    SDL_Surface* Display = nullptr;
    SDL_Thread* thread = nullptr;

    bool OnInit();
    void OnEvent( SDL_Event& Event );
    void OnUpdate();
    static int OnUpdateStatic( void* param );
    void OnRender();
    void OnResize( GLint w, GLint h );
    void OnCleanup();

    void OnKeyDown( SDLKey sym, SDLMod mod, Uint16 unicode );
    void OnKeyUp( SDLKey sym, SDLMod mod, Uint16 unicode );
    void OnMouseClickLeft( GLint X, GLint Y );

    static const Uint32 Time_Interval;
    Uint32 Delay();

    void InitRoadAdditionsGL();

    bool doUpdate = false;
    bool WaitForEnd = false;

    BulletProto Weapons[ 4 ]{};

    static const GLubyte SA_GAMESCREEN = 0;
    static const GLubyte SA_GAMESCREEN_BRIEFING = 1;
    static const GLubyte SA_GAMESCREEN_PAUSED = 2;
    static const GLubyte SA_MISSIONSELECTION = 3;
    static const GLubyte SA_DEADSCREEN = 4;
    static const GLubyte SA_WINSCREEN = 5;
    static const GLubyte SA_CUSTOMIZE = 120;
    static const GLubyte SA_MAINMENU = 100;
    static const GLubyte SA_OPTIONS = 110;

    GLubyte SCREEN = 0;
    GLuint RESOLUTIONS[ 4 ][ 2 ]{};
    GLint current_resolution = 0;

    void ChangeScreen( GLubyte SCR );

    Jet* jet = nullptr;
    Map* map = nullptr;
    std::vector<Enemy*> enemies{};
    std::vector<Enemy*> Egarbage{};
    Font* font_pause_txt = nullptr;
    Font* font_gui_txt = nullptr;
    Font* font_big = nullptr;
    GLuint ButtonTexture = 0;

    GLint current_filtering = 0;
    GLdouble Rotation = 0.0;
    GLint angle = 0;
    bool DynamicCamera = false;
    Uint32 timeS = 0;
    Uint32 nextTick = 0;
    Uint32 nextFrame = 0;

    bool play_sound = false;
    Mix_Chunk* laser = nullptr;
    Mix_Chunk* blaster = nullptr;
    Mix_Chunk* torpedo = nullptr;
    Mix_Chunk* click = nullptr;
    void PlaySound( Mix_Chunk* sound );

    Circle* Radar = nullptr;
    Button btnExit{};
    Button btnChangeFiltering{};
    Button btnSelectMission{};
    Button btnSelectMissionCancel{};
    Button btnReturnToMissionSelection{};
    Button btnReturnToMainMenu{};
    Button btnQuitMission{};
    Button btnStartMission{};
    Button btnGO{};
    Button Options{};

    Button btnNextMap{};
    Button btnPrevMap{};
    Button btnNextJet{};
    Button btnPrevJet{};
    Button btnCustomizeReturn{};
    Button btnCustomize{};

    Button btnWeap1{};
    Button btnWeap2{};
    Button btnWeap3{};
    GLubyte Weap1 = 0;
    GLubyte Weap2 = 0;
    GLubyte Weap3 = 0;

    GLuint HUDtex = 0;

    GLint SCREEN_WIDTH = 0;
    GLint SCREEN_HEIGHT = 0;
    GLint SCREEN_DEPTH = 0;

    GLuint FramesDone = 0;
    GLuint ShotsDone = 0;
    time_t TimePassed{};
    GLuint FPS = 0;
    GLdouble CalculatedFPS = 0.0;
    GLdouble tempFPS = 0.0;
    GLfloat LightAmbient[ 4 ]{};
    GLfloat LightDiffuse[ 4 ]{};
    GLfloat LightPosition[ 4 ]{};

    std::vector<Bullet*> bullet{};
    std::vector<Bullet*> enemybullet{};
    std::vector<Bullet*> Bgarbage{};

    std::vector<MapProto> maps_container{};
    GLuint current_map = 0;

    GLint max_dimention = 0;
    GLint min_dimention = 0;

    GLuint menu_background = 0;
    GLuint menu_background_overlay = 0;
    GLfloat alpha_value = 0.0f;
    bool background_effect_equation = false;

    GLuint starfield_texture = 0;

    GLuint cyber_ring_texture[ 3 ]{};
    GLdouble cyber_ring_rotation[ 3 ]{};
    GLfloat cyber_ring_color[ 3 ][ 4 ]{};
    bool cyber_ring_rotation_direction[ 3 ]{};

    std::vector<ModelProto> jet_models_proto{};
    std::vector<ModelProto> jets_container{};
    Model preview_model{};
    GLuint current_jet = 0;
    GLdouble model_rotation = 0.0;
    std::string LastSelectedJetName{};

    GLdouble speed_anim = 0.0;
    Circle* speed_fan_ring = nullptr;

    GLfloat HUD_Color_4fv[ 3 ][ 4 ]{};
    GLuint HUD_Color = 0.0;

    bool FULLSCREEN = false;

    char HUDMESSAGE[ 48 ]{};

    void Pause();
    void Unpause();

    void GameUpdate();
    void GameUpdatePaused();
    void GameScreen();
    void GameScreenPaused();

    void GameScreenBriefing();
    void GameScreenBriefingUpdate();
    void GameKeyboardBriefingPressed( SDLKey sym );

    void DeadScreen();
    void DeadScreenUpdate();

    void MissionSelectionScreen();
    void MissionSelectionUpdate();

    void UpdateMainMenu();

    void GameKeyboardPressed( SDLKey sym );
    void GameKeyboardUnpressed( SDLKey sym );
    void GameKeyboardPausedPressed( SDLKey sym );
    void GameKeyboardPausedUnpressed( SDLKey sym );

    void DrawBullets();
    void AddBullet( GLuint wID );
    void DrawCrosshair();
    void DrawAxis();
    void setCamera();
    void DrawLine( GLdouble X, GLdouble Y );
    void Retarget();

    void ClearMapData();
    void CreateMapData( MapProto map_data, ModelProto model_data );
    void LoadMapProto();

    void LoadConfig();
    void SaveConfig();

    void LoadJetProto();

    void GoFullscreen( bool& b );
    bool InitNewSurface( GLint W, GLint H, GLint D, bool F );
    void DrawHudRect( GLdouble X, GLdouble Y, GLdouble W, GLdouble H );
    void DrawHUDLine( GLdouble X1, GLdouble Y1, GLdouble X2, GLdouble Y2, GLdouble T );
    void DrawHUDPiece( GLdouble X, GLdouble Y, GLdouble RotAngleZ );
    void DrawHUDBar( const GLuint& X, const GLuint& Y, const GLuint& W, const GLuint& H, const GLuint& Current, const GLuint& Max );
    void DrawPauseText();
    void RenderHUD();
    void Render3D();

    void ScreenCustomize();
    void UpdateCustomize();

    void DrawCyberRings();
    void DrawCyberRingsMini();
    void UpdateCyberRings();

    void WinScreen();
    void WinUpdate();

    void DrawClouds();
    void UpdateClouds();

    void DrawMainMenu();

    void SetOrtho();
    void SetPerspective( const GLdouble& Angle );
};

#endif
