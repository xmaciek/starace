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
    Model* m_previewModel = nullptr;
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

    std::mutex m_mutexJet{};

    HudData m_hudData{};
    Hud m_hud{};
    UIRings m_uiRings{};

    BulletProto m_weapons[ 4 ]{};
    Button m_btnCustomizeReturn{};
    Button m_btnGO{};
    Button m_btnNextJet{};
    Button m_btnNextMap{};
    Button m_btnPrevJet{};
    Button m_btnPrevMap{};
    Button m_btnQuitMission{};
    Button m_btnReturnToMainMenu{};
    Button m_btnSelectMissionCancel{};
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
    std::pmr::map<std::filesystem::path, Texture> m_textures{};

    ScreenTitle m_screenTitle{};
    ScreenWinLoose m_screenWin{};
    ScreenWinLoose m_screenLoose{};

    Targeting m_targeting{};

    glm::vec4 m_currentHudColor = color::winScreen;
    double m_modelRotation = 0.0;
    float m_maxDimention = 0.0f;
    float m_alphaValue = 1.0f;
    float m_alphaN = 0.0f;
    float m_angle = 55.0f;
    uint32_t m_currentJet = 0;
    uint32_t m_currentMap = 0;
    uint32_t m_shotsDone = 0;

    Screen m_currentScreen = Screen::eGame;
    RotaryIndex<> m_weap1{};
    RotaryIndex<> m_weap2{};
    RotaryIndex<> m_weap3{};

    uint32_t viewportHeight() const;
    uint32_t viewportWidth() const;
    float viewportAspect() const;
    void addBullet( uint32_t wID );
    void changeScreen( Screen, Audio::Chunk sound = {} );
    void clearMapData();
    void createMapData( const MapCreateInfo&, const ModelProto& );
    void gameKeyboardBriefingPressed( SDL_Scancode );
    void gameKeyboardPausedPressed( SDL_Scancode );
    void gameKeyboardPausedUnpressed( SDL_Scancode );
    void gameKeyboardPressed( SDL_Scancode );
    void gameKeyboardUnpressed( SDL_Scancode );
    void loadConfig();
    void loadJetProto();
    void loadMapProto();
    void onKeyDown( const SDL_Keysym& );
    void onKeyUp( const SDL_Keysym& );
    void onResize( uint32_t w, uint32_t h );
    void pause();
    void reloadPreviewModel();

    std::tuple<glm::mat4, glm::mat4> getCameraMatrix() const;

    // purposefully copy argument
    void render3D( RenderContext );
    void renderClouds( RenderContext ) const;
    void renderCrosshair( RenderContext );
    void renderDeadScreen( RenderContext );
    void renderGameScreen( RenderContext );
    void renderGameScreenBriefing( RenderContext );
    void renderGameScreenPaused( RenderContext );
    void renderHUD( RenderContext );
    void renderHudTex( RenderContext, const glm::vec4& );
    void renderMainMenu( RenderContext );
    void renderMissionSelectionScreen( RenderContext );
    void renderPauseText( RenderContext );
    void renderScreenCustomize( RenderContext );
    void renderWinScreen( RenderContext );

    void preloadData();
    void retarget();
    void saveConfig();
    void setCamera();
    void unpause();
    void updateClouds( const UpdateContext& );
    void updateCustomize( const UpdateContext& );
    void updateDeadScreen( const UpdateContext& );
    void updateGame( const UpdateContext& );
    void updateGamePaused( const UpdateContext& );
    void updateGameScreenBriefing( const UpdateContext& );
    void updateMainMenu( const UpdateContext& );
    void updateMissionSelection( const UpdateContext& );
    void updateWin( const UpdateContext& );

    virtual void onAction( Action ) override;
    virtual void onInit() override;
    virtual void onExit() override;
    virtual void onRender( RenderContext ) override;
    virtual void onUpdate( const UpdateContext& ) override;
    virtual void onEvent( const SDL_Event& ) override;
    virtual void onMouseEvent( const MouseEvent& ) override;
};
