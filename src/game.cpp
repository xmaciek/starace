#include "game.hpp"

#include "colors.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

Game::Game()
{
    m_radar = new Circle( 48, 64 );

    m_hudColor4fv[ 0 ][ 0 ] = 0.0275f;
    m_hudColor4fv[ 0 ][ 1 ] = 1.0f;
    m_hudColor4fv[ 0 ][ 2 ] = 0.075f;
    m_hudColor4fv[ 0 ][ 3 ] = 1.0f;

    m_hudColor4fv[ 1 ][ 0 ] = 1;
    m_hudColor4fv[ 1 ][ 1 ] = 0.6f;
    m_hudColor4fv[ 1 ][ 2 ] = 0.1f;
    m_hudColor4fv[ 1 ][ 3 ] = 0.8f;

    m_hudColor4fv[ 2 ][ 0 ] = 1;
    m_hudColor4fv[ 2 ][ 1 ] = 0.1f;
    m_hudColor4fv[ 2 ][ 2 ] = 0.1f;
    m_hudColor4fv[ 2 ][ 3 ] = 1.0f;

    changeScreen( Screen::eMainMenu );

    m_enemies.reserve( 100 );
    m_enemyGarbage.reserve( 32 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
    m_bulletGarbage.reserve( 200 );
}

Game::~Game()
{
    clearMapData();

    delete m_fontPauseTxt;
    delete m_fontGuiTxt;
    delete m_fontBig;
    delete m_speedFanRing;
    delete m_radar;

    destroyTexture( m_hudTex );
    destroyTexture( m_buttonTexture );
    destroyTexture( m_menuBackground );
    destroyTexture( m_menuBackgroundOverlay );
    for ( auto it : m_cyberRingTexture ) {
        destroyTexture( it );
    }
    destroyTexture( m_starfieldTexture );
    for ( auto& it : m_mapsContainer ) {
        destroyTexture( it.preview_image );
    }
    delete m_previewModel;
    delete m_renderer;
}

int32_t Game::run()
{
    if ( !onInit() ) {
        return -1;
    }

    m_thread = std::thread( &Game::onUpdate, this );
    SDL_Event Event{};
    while ( m_isRunning ) {
        while ( SDL_PollEvent( &Event ) ) {
            onEvent( Event );
        }
        onRender();
    }
    if ( m_thread.joinable() ) {
        m_thread.join();
    }

    onCleanup();
    return 0;
}

void Game::onEvent( const SDL_Event& event )
{
    switch ( event.type ) {
    case SDL_QUIT:
        m_isRunning = false;
        break;

    case SDL_KEYDOWN:
        onKeyDown( event.key.keysym );
        break;

    case SDL_KEYUP:
        onKeyUp( event.key.keysym );
        break;

    case SDL_WINDOWEVENT:
        switch ( event.window.event ) {
        case SDL_WINDOWEVENT_RESIZED:
            setViewportSize( event.window.data1, event.window.data2 );
            onResize( viewportWidth(), viewportHeight() );
        default:
            break;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if ( event.button.button == SDL_BUTTON_LEFT ) {
            onMouseClickLeft( event.button.x, event.button.y );
        }
        break;
    }
}

void Game::onCleanup()
{
    //   Mix_HaltMusic();
//     Mix_HaltChannel( -1 );
// 
//     std::cout << Mix_GetError() << "\n";
//     Mix_FreeChunk( m_laser );
//     Mix_FreeChunk( m_blaster );
//     Mix_FreeChunk( m_torpedo );
//     Mix_FreeChunk( m_click );
//     Mix_CloseAudio();
// 
//     Mix_Quit();

//     SDL_FreeSurface( m_display );
    SDL_DestroyWindow( m_display );
    SDL_Quit();
    saveConfig();
}

bool Game::onInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) {
        std::cout << "Unable to init SDL\n"
                  << SDL_GetError() << "\n";
        return false;
    }


//     if ( Mix_OpenAudio( 22050, AUDIO_S16SYS, 2, 4096 ) != 0 ) {
//         std::fprintf( stderr, "Unable to initialize audio: %s\n", Mix_GetError() );
//         return false;
//     }
// 
//     m_laser = Mix_LoadWAV( "sounds/laser.wav" );
//     m_blaster = Mix_LoadWAV( "sounds/blaster.wav" );
//     m_torpedo = Mix_LoadWAV( "sounds/torpedo.wav" );
//     m_click = Mix_LoadWAV( "sounds/click.wav" );

    loadConfig();

    m_display = SDL_CreateWindow( "Starace"
        , SDL_WINDOWPOS_UNDEFINED
        , SDL_WINDOWPOS_UNDEFINED
        , static_cast<int>( viewportWidth() )
        , static_cast<int>( viewportHeight() )
        , Renderer::windowFlag()
            | ( m_isFullscreen ? SDL_WINDOW_FULLSCREEN : 0 )
            | SDL_WINDOW_RESIZABLE
    );
    assert( m_display );
    if ( !m_display ) {
        std::cout << "Unable to create window: "
                  << SDL_GetError()
                  << std::endl;
        return false;
    }

    m_renderer = Renderer::create( m_display );
    initRoadAdditions();
    onResize( viewportWidth(), viewportHeight() );
    return true;
}

void Game::onResize( int32_t w, int32_t h )
{
    m_maxDimention = std::max( w, h );
    m_minDimention = std::min( w, h );
    m_renderer->setViewportSize( w, h );

    m_btnExit.setPosition( ( viewportWidth() / 2 ) + 4, viewportHeight() * 0.15 );
    m_btnQuitMission.setPosition( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnChangeFiltering.setPosition( 512, viewportHeight() - 192 );
    m_btnSelectMission.setPosition( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 + 52 );
    m_btnGO.setPosition( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 );
    m_btnStartMission.setPosition( ( viewportWidth() / 2 ) + 4, viewportHeight() * 0.15 );
    m_btnReturnToMainMenu.setPosition( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnReturnToMissionSelection.setPosition( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 );
    m_btnNextMap.setPosition( viewportWidth() - 240, viewportHeight() / 2 - 24 );
    m_btnPrevMap.setPosition( 48, viewportHeight() / 2 - 24 );
    m_btnCustomize.setPosition( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnCustomizeReturn.setPosition( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 + 52 );
    m_btnNextJet.setPosition( viewportWidth() - 240, viewportHeight() / 2 - 24 );
    m_btnPrevJet.setPosition( 48, viewportHeight() / 2 - 24 );

    m_btnWeap1.setPosition( viewportWidth() / 2 - 196 - 96, viewportHeight() * 0.15 + 52 - 76 );
    m_btnWeap2.setPosition( viewportWidth() / 2 - 96, viewportHeight() * 0.15 + 52 - 76 );
    m_btnWeap3.setPosition( viewportWidth() / 2 + 100, viewportHeight() * 0.15 + 52 - 76 );
}

void Game::initRoadAdditions()
{
    m_fontPauseTxt = new Font( "misc/DejaVuSans-Bold.ttf", 18 );
    m_fontGuiTxt = new Font( "misc/DejaVuSans-Bold.ttf", 12 );
    m_fontBig = new Font( "misc/DejaVuSans-Bold.ttf", 32 );

    m_buttonTexture = loadTexture( "textures/button1.tga" );

    m_btnExit = Button( "Exit Game", m_fontGuiTxt, m_buttonTexture );
    m_btnSelectMission = Button( "Select Mission", m_fontGuiTxt, m_buttonTexture );
    m_btnQuitMission = Button( "Quit Mission", m_fontGuiTxt, m_buttonTexture );
    m_btnStartMission = Button( "Start Mission", m_fontGuiTxt, m_buttonTexture );
    m_btnReturnToMissionSelection = Button( "Return", m_fontGuiTxt, m_buttonTexture );
    m_btnReturnToMainMenu = Button( "Return", m_fontGuiTxt, m_buttonTexture );
    m_btnGO = Button( "GO!", m_fontGuiTxt, m_buttonTexture );
    m_btnNextMap = Button( "Next Map", m_fontGuiTxt, m_buttonTexture );

    m_btnPrevMap = Button( "Previous Map", m_fontGuiTxt, m_buttonTexture );
    m_btnNextJet = Button( "Next Jet", m_fontGuiTxt, m_buttonTexture );
    m_btnPrevJet.setEnabled( m_currentJet > 0 );
    m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );

    m_btnPrevJet = Button( "Previous Jet", m_fontGuiTxt, m_buttonTexture );
    m_btnCustomizeReturn = Button( "Done", m_fontGuiTxt, m_buttonTexture );
    m_btnCustomize = Button( "Customize", m_fontGuiTxt, m_buttonTexture );
    m_btnWeap1 = Button( m_fontGuiTxt, m_buttonTexture );
    m_btnWeap2 = Button( m_fontGuiTxt, m_buttonTexture );
    m_btnWeap3 = Button( m_fontGuiTxt, m_buttonTexture );

    switch ( m_weap1 ) {
    case 0:
        m_btnWeap1.setText( "Laser" );
        break;
    case 1:
        m_btnWeap1.setText( "Blaster" );
        break;
    case 2:
        m_btnWeap1.setText( "Torpedo" );
        break;
    }

    switch ( m_weap2 ) {
    case 0:
        m_btnWeap2.setText( "Laser" );
        break;
    case 1:
        m_btnWeap2.setText( "Blaster" );
        break;
    case 2:
        m_btnWeap2.setText( "Torpedo" );
        break;
    }

    switch ( m_weap3 ) {
    case 0:
        m_btnWeap3.setText( "Laser" );
        break;
    case 1:
        m_btnWeap3.setText( "Blaster" );
        break;
    case 2:
        m_btnWeap3.setText( "Torpedo" );
        break;
    }

    m_hudTex = loadTexture( "textures/HUDtex.tga" );

    m_menuBackground = loadTexture( "textures/background.tga" );
    m_menuBackgroundOverlay = loadTexture( "textures/background-overlay.tga" );
    m_starfieldTexture = loadTexture( "textures/star_field_transparent.tga" );
    m_alphaValue = 1;
    m_backgroundEffectEquation = false;

    m_speedFanRing = new Circle( 32, 26 );

    m_cyberRingTexture[ 0 ] = loadTexture( "textures/cyber_ring1.tga" );
    m_cyberRingTexture[ 1 ] = loadTexture( "textures/cyber_ring2.tga" );
    m_cyberRingTexture[ 2 ] = loadTexture( "textures/cyber_ring3.tga" );

    float temp_colors[ 4 ][ 4 ] = { { 1, 1, 1, 0.8 }, { 1, 1, 1, 0.7 }, { 1, 1, 1, 0.6 }, { 1, 1, 1, 0.7 } };

    memcpy( m_cyberRingColor, temp_colors, sizeof( float ) * 12 );

    BulletProto tmpWeapon;
    tmpWeapon.type = Bullet::Type::eSlug;
    tmpWeapon.delay = 0.1;
    tmpWeapon.energy = 15;
    tmpWeapon.damage = 1;
    tmpWeapon.score_per_hit = 1;
    tmpWeapon.color1 = color::yellow;
    tmpWeapon.color2 = color::yellow;
    m_weapons[ 0 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eBlaster;
    tmpWeapon.speed = 16;
    tmpWeapon.damage = 10;
    tmpWeapon.energy = 10;
    tmpWeapon.delay = 0.2;
    tmpWeapon.color1 = color::blaster;
    tmpWeapon.score_per_hit = 30;
    m_weapons[ 1 ] = tmpWeapon;

    tmpWeapon.color1 = color::yellowBlaster;
    tmpWeapon.delay = 0.4;
    m_weapons[ 3 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eTorpedo;
    tmpWeapon.damage = 1;
    tmpWeapon.delay = 0.4;
    tmpWeapon.energy = 1;
    tmpWeapon.speed = 8;
    tmpWeapon.score_per_hit = 2;
    tmpWeapon.color1 = color::orchid;
    m_weapons[ 2 ] = tmpWeapon;
}

void Game::updateCyberRings( const UpdateContext& updateContext )
{
    m_cyberRingRotation[ 0 ] += 25.0f * updateContext.deltaTime;
    m_cyberRingRotation[ 1 ] -= 15.0f * updateContext.deltaTime;
    if ( m_cyberRingRotationDirection[ 2 ] ) {
        m_cyberRingRotation[ 2 ] += 35.0f * updateContext.deltaTime;
    }
    else {
        m_cyberRingRotation[ 2 ] -= 20.0f * updateContext.deltaTime;
    }

    if ( m_cyberRingRotation[ 0 ] >= 360 ) {
        m_cyberRingRotation[ 0 ] -= 360;
    }
    if ( m_cyberRingRotation[ 1 ] < 0 ) {
        m_cyberRingRotation[ 1 ] += 360;
    }
    if ( m_cyberRingRotation[ 2 ] < 0 ) {
        m_cyberRingRotationDirection[ 2 ] = true;
    }
    if ( m_cyberRingRotation[ 2 ] > 90 ) {
        m_cyberRingRotationDirection[ 2 ] = false;
    }
}

void Game::onRender()
{
    m_fpsMeter.frameBegin();
    m_renderer->clear();
    RenderContext rctx{};
    rctx.renderer = m_renderer;
    rctx.projection = glm::ortho<float>( 0.0f, viewportWidth(), 0.0f, viewportHeight(), -100.0f, 100.0f );
    switch ( m_currentScreen ) {
    case Screen::eGame:
        renderGameScreen( rctx );
        break;
    case Screen::eGamePaused:
        renderGameScreenPaused( rctx );
        break;
    case Screen::eGameBriefing:
        renderGameScreenBriefing( rctx );
        break;
    case Screen::eDead:
        renderDeadScreen( rctx );
        break;
    case Screen::eWin:
        renderWinScreen( rctx );
        break;
    case Screen::eMissionSelection:
        renderMissionSelectionScreen( rctx );
        break;
    case Screen::eMainMenu:
        renderMainMenu( rctx );
        break;
    case Screen::eCustomize:
        renderScreenCustomize( rctx );
        break;
    default:
        break;
    }
    m_fpsMeter.frameEnd();
    m_renderer->present();
}

void Game::onUpdate()
{
    const UpdateContext updateContext{ 0.032f };
    while ( m_isRunning ) {
        const std::chrono::time_point tp = std::chrono::steady_clock::now() + std::chrono::milliseconds{ 16 };
        switch ( m_currentScreen ) {
        case Screen::eGame:
            updateGame( updateContext );
            break;
        case Screen::eGamePaused:
            updateGamePaused( updateContext );
            break;
        case Screen::eGameBriefing:
            updateGameScreenBriefing( updateContext );
            break;
        case Screen::eDead:
            updateDeadScreen( updateContext );
            break;
        case Screen::eWin:
            updateWin( updateContext );
            break;
        case Screen::eMissionSelection:
            updateMissionSelection( updateContext );
            break;
        case Screen::eMainMenu:
            updateMainMenu( updateContext );
            break;
        case Screen::eCustomize:
            updateCustomize( updateContext );
            break;
        }
        std::this_thread::sleep_until( tp );
    }
}

void Game::pause()
{
    changeScreen( Screen::eGamePaused );
}

void Game::unpause()
{
    changeScreen( Screen::eGame );
}

void Game::updateGamePaused( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Game::updateGame( const UpdateContext& updateContext )
{
    if ( m_jet->status() == Jet::Status::eDead ) {
        changeScreen( Screen::eDead );
    }
    if ( m_enemies.empty() ) {
        changeScreen( Screen::eWin );
    }
    if ( m_jet->health() <= 20 ) {
        m_hudColor = 2;
    }
    else {
        m_hudColor = 0;
    }

    if ( m_jet->isShooting( 0 ) ) {
        addBullet( 0 );
    }
    if ( m_jet->isShooting( 1 ) ) {
        addBullet( 1 );
    }
    if ( m_jet->isShooting( 2 ) ) {
        addBullet( 2 );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( Enemy*& e : m_enemies ) {
            e->update( updateContext );
            if ( e->isWeaponReady() ) {
                std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
                m_enemyBullets.push_back( e->weapon() );
            }
            if ( e->status() == Enemy::Status::eDead ) {
                m_jet->addScore( e->score(), true );
                m_enemyGarbage.push_back( e );
                e = nullptr;
            }
        }
        m_enemies.erase( std::remove( m_enemies.begin(), m_enemies.end(), nullptr ), m_enemies.end() );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet* it : m_enemyBullets ) {
            it->processCollision( m_jet );
        }
    }

    m_jet->update( updateContext );
    m_speedAnim += m_jet->speed() * 270.0f * updateContext.deltaTime;
    if ( m_speedAnim >= 360 ) {
        m_speedAnim -= 360;
    }
    m_map->setJetData( m_jet->position(), m_jet->velocity() );
    m_map->update( updateContext );

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        std::lock_guard<std::mutex> lg2( m_mutexBullet );
        for ( Bullet*& b : m_bullets ) {
            for ( Enemy* e : m_enemies ) {
                b->processCollision( e );
            }

            b->update( updateContext );
            if ( b->status() == Bullet::Status::eDead ) {
                m_bulletGarbage.push_back( b );
                b = nullptr;
                //         i-=1;
            }
        }
        m_bullets.erase( std::remove( m_bullets.begin(), m_bullets.end(), nullptr ), m_bullets.end() );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet*& b : m_enemyBullets ) {
            b->update( updateContext );
            if ( b->status() == Bullet::Status::eDead ) {
                m_bulletGarbage.push_back( b );
                b = nullptr;
            }
        }
        m_enemyBullets.erase( std::remove( m_enemyBullets.begin(), m_enemyBullets.end(), nullptr ), m_enemyBullets.end() );
    }

    for ( Enemy*& e : m_enemyGarbage ) {
        if ( e->deleteMe() ) {
            delete e;
            e = nullptr;
        }
    }
    m_enemyGarbage.erase( std::remove( m_enemyGarbage.begin(), m_enemyGarbage.end(), nullptr ), m_enemyGarbage.end() );

    for ( Bullet*& b : m_bulletGarbage ) {
        if ( b->deleteMe() ) {
            delete b;
            b = nullptr;
        }
    }
    m_bulletGarbage.erase( std::remove( m_bulletGarbage.begin(), m_bulletGarbage.end(), nullptr ), m_bulletGarbage.end() );

    updateCyberRings( updateContext );
}

void Game::addBullet( uint32_t wID )
{
    if ( !m_jet->isWeaponReady( wID ) ) {
        return;
    }
    m_jet->takeEnergy( wID );
    m_bullets.push_back( m_jet->weapon( wID ) );
    m_shotsDone++;
    switch ( m_bullets.back()->type() ) {
    case Bullet::Type::eBlaster:
        playSound( m_blaster );
        break;
    case Bullet::Type::eSlug:
        playSound( m_laser );
        break;
    case Bullet::Type::eTorpedo:
        playSound( m_torpedo );
        break;
    }
}

void Game::retarget()
{
    std::lock_guard<std::mutex> lg( m_mutexEnemy );
    if ( m_enemies.empty() ) {
        return;
    }
    static std::mt19937_64 random{ std::random_device()() };
    m_jet->lockTarget( m_enemies[ random() % m_enemies.size() ] );
}

void Game::updateMainMenu( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
    updateCyberRings( updateContext );
}

void Game::updateGameScreenBriefing( const UpdateContext& updateContext )
{
    updateGamePaused( updateContext );
}

void Game::clearMapData()
{
    std::cout << "Moving all enemies to garbage.\n";
    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( Enemy* e : m_enemies ) {
            delete e;
        }
        m_enemies.clear();
    }

    std::cout << "Moving all bullets to garbage.\n";
    {
        std::lock_guard<std::mutex> lg( m_mutexBullet );
        for ( Bullet* b : m_bullets ) {
            delete b;
        }
        m_bullets.clear();
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet* b : m_enemyBullets ) {
            delete b;
        }
        m_enemyBullets.clear();
    }

    std::cout << "Cleaning garbage...\n";
    for ( Enemy* e : m_enemyGarbage ) {
        delete e;
    }
    m_enemyGarbage.clear();

    for ( Bullet* b : m_bulletGarbage ) {
        delete b;
    }
    m_bulletGarbage.clear();

    std::cout << "Cleaning garbage: done.\n";
    delete m_map;
    m_map = nullptr;
    delete m_jet;
    m_jet = nullptr;
}

void Game::createMapData( const MapProto& mapData, const ModelProto& modelData )
{
    m_shotsDone = 0;
    m_hudColor = 0;
    m_jet = new Jet( modelData );
    m_map = new Map( mapData );
    m_jet->setWeapon( m_weapons[ m_weap1 ], 0 );
    m_jet->setWeapon( m_weapons[ m_weap2 ], 1 );
    m_jet->setWeapon( m_weapons[ m_weap3 ], 2 );

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        m_enemies.clear();
        m_enemies.reserve( mapData.enemies );
        for ( uint32_t i = 0; i < mapData.enemies; i++ ) {
            m_enemies.push_back( new Enemy() );
            m_enemies.back()->setTarget( m_jet );
            m_enemies.back()->setWeapon( m_weapons[ 3 ] );
        }
    }

    for ( MapProto& it : m_mapsContainer ) {
        destroyTexture( it.preview_image );
        it.preview_image = 0;
    }
}

void Game::updateMissionSelection( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Game::changeScreen( Screen scr )
{
    SDL_ShowCursor( scr != Screen::eGame );

    switch ( scr ) {
    case Screen::eGame:
    case Screen::eGamePaused:
    case Screen::eDead:
    case Screen::eMainMenu:
    case Screen::eWin:
        m_currentScreen = scr;
        break;

    case Screen::eGameBriefing:
        createMapData( m_mapsContainer.at( m_currentMap ), m_jetsContainer.at( m_currentJet ) );
        m_currentScreen = scr;
        break;

    case Screen::eMissionSelection:
        clearMapData();
        m_currentScreen = scr;
        break;

    case Screen::eCustomize:
        m_modelRotation = 135.0;
        reloadPreviewModel();
        m_currentScreen = scr;
        break;

    default:
        break;
    }
}

void Game::goFullscreen( bool b )
{
    b = !b;
}

void Game::loadMapProto()
{
    m_mapsContainer.clear();
    MapProto map;
    std::ifstream MapFile( "maps.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( MapFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "[MAP]" ) == 0 ) {
            m_mapsContainer.push_back( map );
        }
        if ( strcmp( value_1, "name" ) == 0 ) {
            m_mapsContainer.back().name = value_2;
        }
        if ( strcmp( value_1, "enemies" ) == 0 ) {
            m_mapsContainer.back().enemies = atoi( value_2 );
        }
        if ( strcmp( value_1, "top" ) == 0 ) {
            m_mapsContainer.back().TOP = value_2;
        }
        if ( strcmp( value_1, "bottom" ) == 0 ) {
            m_mapsContainer.back().BOTTOM = value_2;
        }
        if ( strcmp( value_1, "left" ) == 0 ) {
            m_mapsContainer.back().LEFT = value_2;
        }
        if ( strcmp( value_1, "right" ) == 0 ) {
            m_mapsContainer.back().RIGHT = value_2;
        }
        if ( strcmp( value_1, "front" ) == 0 ) {
            m_mapsContainer.back().FRONT = value_2;
        }
        if ( strcmp( value_1, "back" ) == 0 ) {
            m_mapsContainer.back().BACK = value_2;
        }
        if ( strcmp( value_1, "preview" ) == 0 ) {
            m_mapsContainer.back().preview_image_location = value_2;
        }
    }
    MapFile.close();
    if ( m_mapsContainer.empty() ) {
        m_mapsContainer.push_back( map );
        m_btnNextMap.setEnabled( false );
    }
    m_currentMap = 0;
    m_btnPrevMap.setEnabled( false );
}

void Game::loadJetProto()
{
    m_jetsContainer.clear();
    ModelProto mod;
    std::ifstream JetFile( "jets.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( JetFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "[JET]" ) == 0 ) {
            m_jetsContainer.push_back( mod );
        }
        if ( strcmp( value_1, "name" ) == 0 ) {
            m_jetsContainer.back().name = value_2;
        }
        if ( strcmp( value_1, "texture" ) == 0 ) {
            m_jetsContainer.back().model_texture = value_2;
        }
        if ( strcmp( value_1, "model" ) == 0 ) {
            m_jetsContainer.back().model_file = value_2;
        }
        if ( strcmp( value_1, "scale" ) == 0 ) {
            m_jetsContainer.back().scale = std::strtof( value_2, nullptr );
        }
    }
    JetFile.close();
    if ( m_jetsContainer.empty() ) {
        std::cout << "no jets\n";
        m_jetsContainer.push_back( mod );
    }

    m_currentJet = 0;
    for ( uint32_t i = 0; i < m_jetsContainer.size(); i++ ) {
        if ( m_lastSelectedJetName == m_jetsContainer.at( i ).name ) {
            m_currentJet = i;
        }
    }
    m_btnPrevJet.setEnabled( m_currentJet > 0 );
    m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
}

void Game::loadConfig()
{
    std::ifstream ConfigFile( "config.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( ConfigFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "width" ) == 0 ) {
            setViewportSize( atoi( value_2 ), viewportHeight() );
        }
        if ( strcmp( value_1, "height" ) == 0 ) {
            setViewportSize( viewportWidth(), atoi( value_2 ) );
        }
        if ( strcmp( value_1, "fullscreen" ) == 0 ) {
            m_isFullscreen = atoi( value_2 ) != 0;
        }
        if ( strcmp( value_1, "jet" ) == 0 ) {
            m_lastSelectedJetName = value_2;
        }
        if ( strcmp( value_1, "weap1" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                m_weap1 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                m_weap1 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                m_weap1 = 2;
            }
        }
        if ( strcmp( value_1, "weap2" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                m_weap2 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                m_weap2 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                m_weap2 = 2;
            }
        }
        if ( strcmp( value_1, "weap3" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                m_weap3 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                m_weap3 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                m_weap3 = 2;
            }
        }
        if ( strcmp( value_1, "sound" ) == 0 ) {
            m_isSoundEnabled = atoi( value_2 ) != 0;
        }
    }
    ConfigFile.close();
    loadMapProto();
    loadJetProto();
}

void Game::saveConfig()
{
    std::ofstream ConfigFile( "config.cfg" );
    ConfigFile << "width " << viewportWidth() << "\n";
    ConfigFile << "height " << viewportHeight() << "\n";
    ConfigFile << "fullscreen " << m_isFullscreen << "\n";
    ConfigFile << "sound " << m_isSoundEnabled << "\n";
    ConfigFile << "jet " << m_jetsContainer.at( m_currentJet ).name << "\n";
    ConfigFile << "weap1 ";
    switch ( m_weap1 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile << "weap2 ";
    switch ( m_weap2 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile << "weap3 ";
    switch ( m_weap3 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile.close();
}

void Game::updateClouds( const UpdateContext& updateContext )
{
    if ( m_backgroundEffectEquation ) {
        m_alphaValue += 0.1 * updateContext.deltaTime;
    }
    else {
        m_alphaValue -= 0.1 * updateContext.deltaTime;
    }

    if ( m_alphaValue >= 1 ) {
        m_backgroundEffectEquation = false;
    }
    if ( m_alphaValue <= 0.2 ) {
        m_backgroundEffectEquation = true;
    }
}

void Game::updateCustomize( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
    updateCyberRings( updateContext );
    m_modelRotation += 30.0 * updateContext.deltaTime;
    if ( m_modelRotation >= 360.0 ) {
        m_modelRotation -= 360.0;
    }
}

void Game::playSound( Mix_Chunk* /*sound*/ ) const
{
    if ( m_isSoundEnabled ) {
//         Mix_Playing( Mix_PlayChannel( -1, sound, 0 ) );
    }
}

void Game::updateWin( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Game::updateDeadScreen( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

double Game::viewportWidth() const
{
    return m_viewportWidth;
}

double Game::viewportHeight() const
{
    return m_viewportHeight;
}

void Game::setViewportSize( double w, double h )
{
    m_viewportWidth = w;
    m_viewportHeight = h;
}

void Game::reloadPreviewModel()
{
    Model* model = new Model();
    model->loadOBJ( m_jetsContainer.at( m_currentJet ).model_file.c_str() );
    model->calculateNormal();
    model->bindTexture( loadTexture( m_jetsContainer.at( m_currentJet ).model_texture.c_str() ) );
    {
        std::lock_guard<std::mutex> lg{ m_mutexPreviewModel };
        std::swap( model, m_previewModel );
    }
    delete model;
}
