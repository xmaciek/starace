#include "road.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>

Road::Road()
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

Road::~Road()
{
    clearMapData();

    delete m_fontPauseTxt;
    delete m_fontGuiTxt;
    delete m_fontBig;
    delete m_speedFanRing;
    delete m_radar;

    glDeleteTextures( 1, &m_hudTex );
    glDeleteTextures( 1, &m_buttonTexture );
    glDeleteTextures( 1, &m_menuBackground );
    glDeleteTextures( 1, &m_menuBackgroundOverlay );
    glDeleteTextures( 3, m_cyberRingTexture );
    glDeleteTextures( 1, &m_starfieldTexture );
    for ( auto& it : m_mapsContainer ) {
        glDeleteTextures( 1, &it.preview_image );
    }
}

int32_t Road::run()
{
    if ( !onInit() ) {
        return -1;
    }

    m_thread = std::thread( &Road::onUpdate, this );
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

void Road::onEvent( const SDL_Event& event )
{
    switch ( event.type ) {
    case SDL_QUIT:
        m_isRunning = false;
        break;

    case SDL_KEYDOWN:
        onKeyDown( event.key.keysym.sym, event.key.keysym.mod, event.key.keysym.unicode );
        break;

    case SDL_KEYUP:
        onKeyUp( event.key.keysym.sym, event.key.keysym.mod, event.key.keysym.unicode );
        break;

    case SDL_VIDEORESIZE:
        setViewportSize( event.resize.w, event.resize.h );
        initNewSurface( viewportWidth(), viewportHeight(), 32, m_isFullscreen );
        onResize( viewportWidth(), viewportHeight() );
        break;

    case SDL_MOUSEBUTTONDOWN:
        if ( event.button.button == SDL_BUTTON_LEFT ) {
            onMouseClickLeft( event.button.x, event.button.y );
        }
        break;
    }
}

void Road::onCleanup()
{
    //   Mix_HaltMusic();
    Mix_HaltChannel( -1 );

    std::cout << Mix_GetError() << "\n";
    Mix_FreeChunk( m_laser );
    Mix_FreeChunk( m_blaster );
    Mix_FreeChunk( m_torpedo );
    Mix_FreeChunk( m_click );
    Mix_CloseAudio();

    Mix_Quit();

    SDL_FreeSurface( m_display );
    SDL_Quit();
    saveConfig();
}

bool Road::onInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ) {
        std::cout << "Unable to init SDL\n"
                  << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );

    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0 );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 2 );

    if ( Mix_OpenAudio( 22050, AUDIO_S16SYS, 2, 4096 ) != 0 ) {
        std::fprintf( stderr, "Unable to initialize audio: %s\n", Mix_GetError() );
        return false;
    }

    m_laser = Mix_LoadWAV( "sounds/laser.wav" );
    m_blaster = Mix_LoadWAV( "sounds/blaster.wav" );
    m_torpedo = Mix_LoadWAV( "sounds/torpedo.wav" );
    m_click = Mix_LoadWAV( "sounds/click.wav" );

    loadConfig();

    if ( !initNewSurface( viewportWidth(), viewportHeight(), 32, m_isFullscreen ) ) {
        return false;
    }

    initRoadAdditionsGL();
    onResize( viewportWidth(), viewportHeight() );
    return true;
}

void Road::onResize( int32_t w, int32_t h )
{
    if ( w > h ) {
        m_maxDimention = w;
        m_minDimention = h;
    }
    else {
        m_maxDimention = h;
        m_minDimention = w;
    }

    glViewport( 0, 0, w, h );
    m_btnExit.updateCoord( ( viewportWidth() / 2 ) + 4, viewportHeight() * 0.15 );
    m_btnQuitMission.updateCoord( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnChangeFiltering.updateCoord( 512, viewportHeight() - 192 );
    m_btnSelectMission.updateCoord( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 + 52 );
    m_btnGO.updateCoord( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 );
    m_btnStartMission.updateCoord( ( viewportWidth() / 2 ) + 4, viewportHeight() * 0.15 );
    m_btnReturnToMainMenu.updateCoord( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnReturnToMissionSelection.updateCoord( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 );
    m_btnNextMap.updateCoord( viewportWidth() - 240, viewportHeight() / 2 - 24 );
    m_btnPrevMap.updateCoord( 48, viewportHeight() / 2 - 24 );
    m_btnCustomize.updateCoord( ( viewportWidth() / 2 ) - 196, viewportHeight() * 0.15 );
    m_btnCustomizeReturn.updateCoord( ( viewportWidth() / 2 ) - 96, viewportHeight() * 0.15 + 52 );
    m_btnNextJet.updateCoord( viewportWidth() - 240, viewportHeight() / 2 - 24 );
    m_btnPrevJet.updateCoord( 48, viewportHeight() / 2 - 24 );

    m_btnWeap1.updateCoord( viewportWidth() / 2 - 196 - 96, viewportHeight() * 0.15 + 52 - 76 );
    m_btnWeap2.updateCoord( viewportWidth() / 2 - 96, viewportHeight() * 0.15 + 52 - 76 );
    m_btnWeap3.updateCoord( viewportWidth() / 2 + 100, viewportHeight() * 0.15 + 52 - 76 );
}

void Road::initRoadAdditionsGL()
{
    //   init functons

    if ( TTF_Init() < 0 ) {
        std::cout << "Unable to initialize library: " << TTF_GetError() << "\n";
    }
    //     setTextureFiltering(FILTERING_ANISOTROPIC_X16);
    m_fontPauseTxt = new Font( "misc/DejaVuSans-Bold.ttf", 18 );
    m_fontGuiTxt = new Font( "misc/DejaVuSans-Bold.ttf", 12 );
    m_fontBig = new Font( "misc/DejaVuSans-Bold.ttf", 32 );
    TTF_Quit();

    m_buttonTexture = loadTexture( "textures/button1.tga" );

    m_btnChangeFiltering.setFont( m_fontGuiTxt );
    m_btnChangeFiltering.setText( "Anisotropic x16" );
    m_btnChangeFiltering.setTexture( m_buttonTexture );

    m_btnExit.setFont( m_fontGuiTxt );
    m_btnExit.setText( "Exit Game" );
    m_btnExit.setTexture( m_buttonTexture );

    m_btnSelectMission.setFont( m_fontGuiTxt );
    m_btnSelectMission.setText( "Select Mission" );
    m_btnSelectMission.setTexture( m_buttonTexture );

    m_btnQuitMission.setFont( m_fontGuiTxt );
    m_btnQuitMission.setText( "Quit Mission" );
    m_btnQuitMission.setTexture( m_buttonTexture );

    m_btnStartMission.setFont( m_fontGuiTxt );
    m_btnStartMission.setText( "Start Mission" );
    m_btnStartMission.setTexture( m_buttonTexture );

    m_btnReturnToMissionSelection.setFont( m_fontGuiTxt );
    m_btnReturnToMissionSelection.setText( "Return" );
    m_btnReturnToMissionSelection.setTexture( m_buttonTexture );

    m_btnReturnToMainMenu.setFont( m_fontGuiTxt );
    m_btnReturnToMainMenu.setText( "Return" );
    m_btnReturnToMainMenu.setTexture( m_buttonTexture );

    m_btnGO.setFont( m_fontGuiTxt );
    m_btnGO.setText( "GO!" );
    m_btnGO.setTexture( m_buttonTexture );

    m_btnNextMap.setFont( m_fontGuiTxt );
    m_btnNextMap.setText( "Next Map" );
    m_btnNextMap.setTexture( m_buttonTexture );

    m_btnPrevMap.setFont( m_fontGuiTxt );
    m_btnPrevMap.setText( "Previous Map" );
    m_btnPrevMap.setTexture( m_buttonTexture );

    m_btnNextJet.setFont( m_fontGuiTxt );
    m_btnNextJet.setText( "Next Jet" );
    m_btnNextJet.setTexture( m_buttonTexture );

    m_btnPrevJet.setFont( m_fontGuiTxt );
    m_btnPrevJet.setText( "Previous Jet" );
    m_btnPrevJet.setTexture( m_buttonTexture );

    m_btnCustomizeReturn.setFont( m_fontGuiTxt );
    m_btnCustomizeReturn.setText( "Done" );
    m_btnCustomizeReturn.setTexture( m_buttonTexture );

    m_btnCustomize.setFont( m_fontGuiTxt );
    m_btnCustomize.setText( "Customise" );
    m_btnCustomize.setTexture( m_buttonTexture );

    m_btnWeap1.setFont( m_fontGuiTxt );
    m_btnWeap1.setTexture( m_buttonTexture );

    m_btnWeap2.setFont( m_fontGuiTxt );
    m_btnWeap2.setTexture( m_buttonTexture );

    m_btnWeap3.setFont( m_fontGuiTxt );
    m_btnWeap3.setTexture( m_buttonTexture );

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

    m_timePassed = time( nullptr );
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
    //     m_cyberRingTexture[3] = LoadTexture("textures/cyber_ring4.tga");
    m_cyberRingRotation[ 0 ] = 0;
    m_cyberRingRotation[ 1 ] = 0;
    m_cyberRingRotation[ 2 ] = 0;
    m_cyberRingRotationDirection[ 0 ] = false;
    m_cyberRingRotationDirection[ 1 ] = false;
    m_cyberRingRotationDirection[ 2 ] = false;

    float temp_colors[ 4 ][ 4 ] = { { 1, 1, 1, 0.8 }, { 1, 1, 1, 0.7 }, { 1, 1, 1, 0.6 }, { 1, 1, 1, 0.7 } };

    memcpy( m_cyberRingColor, temp_colors, sizeof( float ) * 12 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    m_lightAmbient[ 0 ] = 0.5f;
    m_lightAmbient[ 1 ] = 0.5f;
    m_lightAmbient[ 2 ] = 0.5f;
    m_lightAmbient[ 3 ] = 1.0f;

    m_lightDiffuse[ 0 ] = 0.8f;
    m_lightDiffuse[ 1 ] = 0.8f;
    m_lightDiffuse[ 2 ] = 0.8f;
    m_lightDiffuse[ 3 ] = 1.0f;

    m_lightPosition[ 0 ] = 0;
    m_lightPosition[ 1 ] = 1;
    m_lightPosition[ 2 ] = 1;
    m_lightPosition[ 3 ] = 1;

    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CCW );
    glMaterialfv( GL_FRONT, GL_AMBIENT, m_lightAmbient );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, m_lightDiffuse );
    glLightfv( GL_LIGHT0, GL_AMBIENT, m_lightAmbient );

    glLightfv( GL_LIGHT0, GL_DIFFUSE, m_lightDiffuse );
    glLightfv( GL_LIGHT0, GL_POSITION, m_lightPosition );
    glEnable( GL_LIGHT0 );

    float fogColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glFogi( GL_FOG_MODE, GL_LINEAR );
    glFogfv( GL_FOG_COLOR, fogColor );
    glFogf( GL_FOG_DENSITY, 0.1f );
    glHint( GL_FOG_HINT, GL_NICEST );
    glFogf( GL_FOG_START, 10.0f );
    glFogf( GL_FOG_END, 25.0f );

    float tempcolor[ 4 ][ 4 ] = {
        { 0.3, 0.8, 1, 1 }, { 1, 0.8, 0.1, 1 }, { 1, 0.3, 0.8, 1 }, { 1, 1, 0, 1 }
    };
    BulletProto tmpWeapon;
    tmpWeapon.type = Bullet::Type::eSlug;
    tmpWeapon.delay = 0.1;
    tmpWeapon.energy = 15;
    tmpWeapon.damage = 1;
    tmpWeapon.score_per_hit = 1;
    memcpy( tmpWeapon.color1, tempcolor[ 3 ], 4 * sizeof( float ) );
    memcpy( tmpWeapon.color2, tempcolor[ 1 ], 4 * sizeof( float ) );
    m_weapons[ 0 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eBlaster;
    tmpWeapon.speed = 16;
    tmpWeapon.damage = 10;
    tmpWeapon.energy = 10;
    tmpWeapon.delay = 0.2;
    memcpy( tmpWeapon.color1, tempcolor[ 0 ], 4 * sizeof( float ) );
    tmpWeapon.score_per_hit = 30;
    m_weapons[ 1 ] = tmpWeapon;

    memcpy( tmpWeapon.color1, tempcolor[ 1 ], 4 * sizeof( float ) );
    tmpWeapon.delay = 0.4;
    m_weapons[ 3 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eTorpedo;
    tmpWeapon.damage = 1;
    tmpWeapon.delay = 0.4;
    tmpWeapon.energy = 1;
    tmpWeapon.speed = 8;
    tmpWeapon.score_per_hit = 2;
    memcpy( tmpWeapon.color1, tempcolor[ 2 ], 4 * sizeof( float ) );
    m_weapons[ 2 ] = tmpWeapon;
}

void Road::updateCyberRings( const UpdateContext& updateContext )
{
    m_cyberRingRotation[ 0 ] += 25.0 * updateContext.deltaTime;
    m_cyberRingRotation[ 1 ] -= 15.0 * updateContext.deltaTime;
    if ( m_cyberRingRotationDirection[ 2 ] ) {
        m_cyberRingRotation[ 2 ] += 35.0 * updateContext.deltaTime;
    }
    else {
        m_cyberRingRotation[ 2 ] -= 20.0 * updateContext.deltaTime;
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

void Road::onRender()
{
    m_timeS = SDL_GetTicks();
    switch ( m_currentScreen ) {
    case Screen::eGame:
        renderGameScreen();
        break;
    case Screen::eGamePaused:
        renderGameScreenPaused();
        break;
    case Screen::eGameBriefing:
        renderGameScreenBriefing();
        break;
    case Screen::eDead:
        renderDeadScreen();
        break;
    case Screen::eWin:
        renderWinScreen();
        break;
    case Screen::eMissionSelection:
        renderMissionSelectionScreen();
        break;
    case Screen::eMainMenu:
        renderMainMenu();
        break;
    case Screen::eCustomize:
        renderScreenCustomize();
        break;
    default:
        break;
    }
    SDL_GL_SwapBuffers();
}

void Road::onUpdate()
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

void Road::pause()
{
    changeScreen( Screen::eGamePaused );
}

void Road::unpause()
{
    changeScreen( Screen::eGame );
}

void Road::updateGamePaused( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Road::updateGame( const UpdateContext& updateContext )
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

void Road::addBullet( uint32_t wID )
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

void Road::onMouseClickLeft( int32_t x, int32_t y )
{
    y = viewportHeight() - y;
    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        if ( m_btnQuitMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eDead );
            break;
        }

    case Screen::eMainMenu:
        if ( m_btnSelectMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMissionSelection );
            break;
        }
        if ( m_btnExit.isClicked( x, y ) ) {
            playSound( m_click );
            m_isRunning = false;
            break;
        }
        if ( m_btnCustomize.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eCustomize );
            break;
        }
        break;
    case Screen::eMissionSelection:
        if ( m_btnStartMission.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eGameBriefing );
            break;
        }
        if ( m_btnReturnToMainMenu.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        if ( m_btnNextMap.isClicked( x, y ) ) {
            m_currentMap++;
            if ( m_currentMap == m_mapsContainer.size() - 1 ) {
                m_btnNextMap.setEnabled( false );
            }
            m_btnPrevMap.setEnabled( true );
            playSound( m_click );
            break;
        }
        if ( m_btnPrevMap.isClicked( x, y ) ) {
            m_currentMap--;
            if ( m_currentMap == 0 ) {
                m_btnPrevMap.setEnabled( false );
            }
            if ( m_mapsContainer.size() > 1 ) {
                m_btnNextMap.setEnabled( true );
            }
            playSound( m_click );
            break;
        }
        break;
    case Screen::eDead:
        if ( m_btnReturnToMissionSelection.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMissionSelection );
            break;
        }
    case Screen::eWin:
        if ( m_btnReturnToMissionSelection.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMissionSelection );
            break;
        }
        break;
    case Screen::eGameBriefing:
        if ( m_btnGO.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eGame );
            break;
        }
        break;
    case Screen::eCustomize:
        if ( m_btnNextJet.isClicked( x, y ) ) {
            playSound( m_click );
            m_currentJet++;
            if ( m_currentJet == m_jetsContainer.size() - 1 ) {
                m_btnNextJet.setEnabled( false );
            }
            m_btnPrevJet.setEnabled( true );
            m_previewModel.loadOBJ( m_jetsContainer.at( m_currentJet ).model_file.c_str() );
            m_previewModel.calculateNormal();
            m_previewModel.bindTexture( loadTexture( m_jetsContainer.at( m_currentJet ).model_texture.c_str() ) );
            break;
        }
        if ( m_btnPrevJet.isClicked( x, y ) ) {
            playSound( m_click );
            m_currentJet--;
            if ( m_currentJet == 0 ) {
                m_btnPrevJet.setEnabled( false );
            }
            if ( m_jetsContainer.size() > 1 ) {
                m_btnNextJet.setEnabled( true );
            }
            m_previewModel.loadOBJ( m_jetsContainer.at( m_currentJet ).model_file.c_str() );
            m_previewModel.calculateNormal();
            m_previewModel.bindTexture( loadTexture( m_jetsContainer.at( m_currentJet ).model_texture.c_str() ) );
            break;
        }
        if ( m_btnCustomizeReturn.isClicked( x, y ) ) {
            playSound( m_click );
            changeScreen( Screen::eMainMenu );
            break;
        }
        if ( m_btnWeap1.isClicked( x, y ) ) {
            playSound( m_click );
            m_weap1++;
            if ( m_weap1 == 3 ) {
                m_weap1 = 0;
            }
            if ( m_weap1 == 0 ) {
                m_btnWeap1.setText( "Laser" );
            }
            if ( m_weap1 == 1 ) {
                m_btnWeap1.setText( "Blaster" );
            }
            if ( m_weap1 == 2 ) {
                m_btnWeap1.setText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap2.isClicked( x, y ) ) {
            playSound( m_click );
            m_weap2++;
            if ( m_weap2 == 3 ) {
                m_weap2 = 0;
            }
            if ( m_weap2 == 0 ) {
                m_btnWeap2.setText( "Laser" );
            }
            if ( m_weap2 == 1 ) {
                m_btnWeap2.setText( "Blaster" );
            }
            if ( m_weap2 == 2 ) {
                m_btnWeap2.setText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap3.isClicked( x, y ) ) {
            playSound( m_click );
            m_weap3++;
            if ( m_weap3 == 3 ) {
                m_weap3 = 0;
            }
            if ( m_weap3 == 0 ) {
                m_btnWeap3.setText( "Laser" );
            }
            if ( m_weap3 == 1 ) {
                m_btnWeap3.setText( "Blaster" );
            }
            if ( m_weap3 == 2 ) {
                m_btnWeap3.setText( "Torpedo" );
            }
            break;
        }
        break;
    default:
        break;
    }
}

void Road::retarget()
{
    std::lock_guard<std::mutex> lg( m_mutexEnemy );
    if ( m_enemies.empty() ) {
        return;
    }
    static std::mt19937_64 random{ std::random_device()() };
    m_jet->lockTarget( m_enemies[ random() % m_enemies.size() ] );
}

void Road::updateMainMenu( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
    updateCyberRings( updateContext );
}

void Road::updateGameScreenBriefing( const UpdateContext& updateContext )
{
    updateGamePaused( updateContext );
}

void Road::clearMapData()
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

void Road::createMapData( const MapProto& mapData, const ModelProto& modelData )
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
        glDeleteTextures( 1, &it.preview_image );
        it.preview_image = 0;
    }
}

void Road::updateMissionSelection( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Road::changeScreen( Screen scr )
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
        m_previewModel.loadOBJ( m_jetsContainer.at( m_currentJet ).model_file.c_str() );
        m_previewModel.bindTexture( loadTexture( m_jetsContainer.at( m_currentJet ).model_texture.c_str() ) );
        m_previewModel.calculateNormal();
        m_currentScreen = scr;
        break;

    default:
        break;
    }
}

void Road::goFullscreen( bool b )
{
    b = !b;
    initNewSurface( viewportWidth(), viewportHeight(), 32, b );
}

bool Road::initNewSurface( int32_t w, int32_t h, int32_t d, bool f )
{
    SDL_Surface* tmp = m_display;
    SDL_Surface* tmp2 = nullptr;
    if ( f ) {
        tmp2 = SDL_SetVideoMode( w, h, d, SDL_DOUBLEBUF | SDL_OPENGL | SDL_FULLSCREEN );
    }
    else {
        tmp2 = SDL_SetVideoMode( w, h, d, SDL_DOUBLEBUF | SDL_OPENGL | SDL_RESIZABLE );
    }
    if ( !tmp2 ) {
        std::cout << "Unable to create display surface:\n";
        std::string error( SDL_GetError() );
        std::cout << error << "\n";
        return false;
    }
    m_display = tmp2;
    if ( tmp ) {
        SDL_FreeSurface( tmp );
    }
    return true;
}

void Road::loadMapProto()
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

void Road::loadJetProto()
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
    if ( m_jetsContainer.size() == 1 ) {
        m_btnNextJet.setEnabled( false );
    }
    m_currentJet = 0;
    for ( uint32_t i = 0; i < m_jetsContainer.size(); i++ ) {
        if ( m_lastSelectedJetName == m_jetsContainer.at( i ).name ) {
            m_currentJet = i;
        }
    }
    if ( m_currentJet == 0 ) {
        m_btnPrevJet.setEnabled( false );
    }
    if ( m_currentJet == m_jetsContainer.size() - 1 ) {
        m_btnNextJet.setEnabled( false );
    }
}

void Road::loadConfig()
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

void Road::saveConfig()
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

void Road::updateClouds( const UpdateContext& updateContext )
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

void Road::setOrtho() const
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, viewportWidth(), 0, viewportHeight(), -192, 192 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void Road::setPerspective( double a ) const
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( a, viewportWidth() / viewportHeight(), 0.001, 2000 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void Road::updateCustomize( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
    updateCyberRings( updateContext );
    m_modelRotation += 30.0 * updateContext.deltaTime;
    if ( m_modelRotation >= 360.0 ) {
        m_modelRotation -= 360.0;
    }
}

void Road::playSound( Mix_Chunk* sound ) const
{
    if ( m_isSoundEnabled ) {
        Mix_Playing( Mix_PlayChannel( -1, sound, 0 ) );
    }
}

void Road::updateWin( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

void Road::updateDeadScreen( const UpdateContext& updateContext )
{
    updateCyberRings( updateContext );
}

double Road::viewportWidth() const
{
    return m_viewportWidth;
}

double Road::viewportHeight() const
{
    return m_viewportHeight;
}

void Road::setViewportSize( double w, double h )
{
    m_viewportWidth = w;
    m_viewportHeight = h;
}
