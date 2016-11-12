#ifndef StarAce_ROAD
#define StarAce_ROAD

#include "SA.h"

#include "Jet.h"
#include "Bullet.h"
#include "Map.h"
#include "Texture.h"
#include "Font.h"
#include "Button.h"
#include "Enemy.h"

#define TAB 9
#define ESC 27


class Road {
public:
  Road();
  ~Road();
  
  
  GLint OnExecute();
  
  
  
private:
  bool Running;
  SDL_Surface* Display;
  SDL_Thread* thread;
  
  bool OnInit();
  void OnEvent(SDL_Event &Event);
  void OnUpdate();
  static int OnUpdateStatic(void *param);
  void OnRender();
  void OnResize(GLint w, GLint h);
  void OnCleanup();
  
  void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
  void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);
  void OnMouseClickLeft(GLint X, GLint Y);
  
  static const Uint32 Time_Interval;
  Uint32 Delay();
  
  void InitRoadAdditionsGL();
  
  bool doUpdate,WaitForEnd;

  BulletProto Weapons[4];
  
  static const GLubyte SA_GAMESCREEN = 0;
  static const GLubyte SA_GAMESCREEN_BRIEFING = 1;
  static const GLubyte SA_GAMESCREEN_PAUSED = 2;
  static const GLubyte SA_MISSIONSELECTION = 3;
  static const GLubyte SA_DEADSCREEN = 4;
  static const GLubyte SA_WINSCREEN = 5;
  static const GLubyte SA_CUSTOMIZE = 120;
  static const GLubyte SA_MAINMENU = 100;
  static const GLubyte SA_OPTIONS = 110;
  
  GLubyte SCREEN;
  GLuint RESOLUTIONS[4][2];
  GLint current_resolution;
  
  void ChangeScreen(GLubyte SCR);
  
  Jet *jet;
  Map *map;
  vector<Enemy*> enemies, Egarbage;
  Font *font_pause_txt;
  Font *font_gui_txt;
  Font *font_big;
  GLuint ButtonTexture;
  
  
  GLint current_filtering;
  
  GLdouble Rotation;
  GLint angle;
  GLint drawing_i, update_i, update_i2;
  bool DynamicCamera;
  Uint32 timeS, nextTick, nextFrame;
  
  bool play_sound;
  Mix_Chunk *laser, *blaster, *torpedo, *click;
  void PlaySound(Mix_Chunk *sound);
  
  Circle *Radar;
  Button btnExit;
  Button btnChangeFiltering;
  Button btnSelectMission;
  Button btnSelectMissionCancel;
  Button btnReturnToMissionSelection;
  Button btnReturnToMainMenu;
  Button btnQuitMission;
  Button btnStartMission;
  Button btnGO;
  Button Options;
  
  Button btnNextMap, btnPrevMap;
  Button btnNextJet, btnPrevJet, btnCustomizeReturn;
  Button btnCustomize;
  
  Button btnWeap1, btnWeap2, btnWeap3;
  GLubyte Weap1, Weap2, Weap3;
  
  GLuint HUDtex;
  
  GLint SCREEN_WIDTH;
  GLint SCREEN_HEIGHT;
  GLint SCREEN_DEPTH;
  
  GLuint FramesDone;
  GLuint ShotsDone;
  time_t TimePassed;
  GLuint FPS;
  GLdouble CalculatedFPS,tempFPS;
  GLfloat LightAmbient[4];
  GLfloat LightDiffuse[4];
  GLfloat LightPosition[4];

  vector<Bullet*> bullet, enemybullet, Bgarbage;

    std::vector<Map> m_maps;
    std::vector<Map>::iterator m_currentMap;
  vector<MapProto> maps_container;
  GLuint current_map;
  
  GLint max_dimention, min_dimention;
  
    Texture m_menuBackground, m_menuBackgroundOverlay;
    Texture m_menuBackgroundStarField;
  
    Texture m_ringTextureA, m_ringTextureB, m_ringTextureC;
  GLdouble cyber_ring_rotation[3];
  GLfloat cyber_ring_color[3][4];
  bool cyber_ring_rotation_direction[3];
  
  vector<ModelProto> jet_models_proto;
  vector<ModelProto> jets_container;
  Model preview_model;
  GLuint current_jet;
  GLdouble model_rotation;
  string LastSelectedJetName;
  
  GLdouble speed_anim;
  Circle *speed_fan_ring;
  
  GLfloat HUD_Color_4fv[3][4];
  GLuint HUD_Color;

  
  bool FULLSCREEN;

  
  char HUDMESSAGE[48];
  
  void Pause();
  void Unpause();
  
  void GameUpdate();
  void GameUpdatePaused();
  void GameScreen();
  void GameScreenPaused();
  
  void GameScreenBriefing();
  void GameScreenBriefingUpdate();
  void GameKeyboardBriefingPressed(SDLKey sym);
  
  void DeadScreen();
  void DeadScreenUpdate();
  
  void MissionSelectionScreen();
  void MissionSelectionUpdate();
  
  
  void UpdateMainMenu();
  
  void GameKeyboardPressed(SDLKey sym);
  void GameKeyboardUnpressed(SDLKey sym); 
  void GameKeyboardPausedPressed(SDLKey sym);
  void GameKeyboardPausedUnpressed(SDLKey sym); 
  
  void DrawBullets();
  void AddBullet(GLuint wID);
  void DrawCrosshair();
  void DrawAxis();
  void setCamera();
  void DrawLine(GLdouble X, GLdouble Y);
  void Retarget();
    
  void ClearMapData();
  void CreateMapData( ModelProto model_data);
  void LoadMapProto();
  
  void LoadConfig();
  void SaveConfig();
  
  void LoadJetProto();
  
  void GoFullscreen(bool &b);
  bool InitNewSurface(GLint W, GLint H, GLint D, bool F);
  void DrawHudRect(GLdouble X, GLdouble Y, GLdouble W, GLdouble H); 
  void DrawHUDLine(GLdouble X1, GLdouble Y1, GLdouble X2, GLdouble Y2, GLdouble T);
  void DrawHUDPiece(GLdouble X, GLdouble Y, GLdouble RotAngleZ);
  void DrawHUDBar(const GLuint &X, const GLuint &Y, const GLuint &W, const GLuint &H, const GLuint &Current, const GLuint &Max);
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
  void SetPerspective( double angle );
    
};

#endif
