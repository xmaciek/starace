#include "game.hpp"

#include "colors.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <cmath>

Game::Game()
{
    m_io = std::make_unique<asyncio::Service>();
    m_io->enqueue( "textures/button1.tga" );
    m_io->enqueue( "textures/HUDtex.tga" );
    m_io->enqueue( "textures/background.tga" );
    m_io->enqueue( "textures/background-overlay.tga" );
    m_io->enqueue( "textures/star_field_transparent.tga" );
    m_io->enqueue( "textures/cyber_ring1.tga" );
    m_io->enqueue( "textures/cyber_ring2.tga" );
    m_io->enqueue( "textures/cyber_ring3.tga" );

    changeScreen( Screen::eMainMenu );

    m_enemies.reserve( 100 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
}

Game::~Game()
{
    clearMapData();

    delete m_fontPauseTxt;
    delete m_fontGuiTxt;
    delete m_fontBig;

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
    delete m_enemyModel;
    delete m_renderer;
    delete m_audio;
    SDL_DestroyWindow( m_display );
    SDL_Quit();
}

int32_t Game::run()
{
    if ( !onInit() ) {
        return -1;
    }

    m_thread = std::thread( &Game::loopGame, this );
    SDL_Event event{};
    while ( m_isRunning ) {
        while ( SDL_PollEvent( &event ) ) {
            std::scoped_lock lock{ m_eventsBottleneck };
            m_events.emplace_back( event );
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
    if ( m_thread.joinable() ) {
        m_thread.join();
    }

    onCleanup();
    return 0;
}

void Game::loopGame()
{
    UpdateContext updateContext{ .deltaTime = 0.0166f };
    while ( m_isRunning ) {
        const std::chrono::time_point tp = std::chrono::steady_clock::now();
        m_fpsMeter.frameBegin();

        m_renderer->beginFrame();
        onRender();
        m_renderer->submit();

        onUpdate( updateContext );
        std::vector<SDL_Event> events{};
        {
            std::scoped_lock lock{ m_eventsBottleneck };
            events = std::move( m_events );
        }
        for ( SDL_Event& it : events ) {
            onEvent( it );
        }
        m_fpsMeter.frameEnd();
        m_renderer->present();

        const std::chrono::time_point now = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::microseconds>( now - tp );
        updateContext.deltaTime = (float)dt.count();
        // TODO: fix game speed later
        updateContext.deltaTime /= 500'000;
    }
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
    case SDL_MOUSEMOTION:
        onMouseMove( event.motion );
        break;
    }
}

void Game::onCleanup()
{
    saveConfig();
}

bool Game::onInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) {
        std::cout << "Unable to init SDL\n"
                  << SDL_GetError() << "\n";
        return false;
    }

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
    m_audio = audio::Engine::create();
    m_laser = m_audio->load( "sounds/laser.wav" );
    m_blaster = m_audio->load( "sounds/blaster.wav" );
    m_torpedo = m_audio->load( "sounds/torpedo.wav" );
    m_click = m_audio->load( "sounds/click.wav" );

    initRoadAdditions();
    onResize( viewportWidth(), viewportHeight() );
    return true;
}

void Game::onResize( uint32_t w, uint32_t h )
{
    m_maxDimention = std::max( w, h );
    m_minDimention = std::min( w, h );
    m_renderer->setViewportSize( w, h );

    const uint32_t halfW = viewportWidth() >> 1;
    const uint32_t halfH = viewportHeight() >> 1;
    const uint32_t h015 = (uint32_t)( (float)viewportHeight() * 0.15f );
    m_btnExit.setPosition( halfW + 4, h015 );
    m_btnQuitMission.setPosition( halfW - 196, h015 );
    m_btnChangeFiltering.setPosition( 512, viewportHeight() - 192 );
    m_btnSelectMission.setPosition( halfW - 96, h015 + 52 );
    m_btnGO.setPosition( halfW - 96, h015 );
    m_btnStartMission.setPosition( halfW + 4, h015 );
    m_btnReturnToMainMenu.setPosition( halfW - 196, h015 );
    m_btnReturnToMissionSelection.setPosition( halfW - 96, h015 );
    m_btnNextMap.setPosition( viewportWidth() - 240, halfH - 24 );
    m_btnPrevMap.setPosition( 48, halfH - 24 );
    m_btnCustomize.setPosition( halfW - 196, h015 );
    m_btnCustomizeReturn.setPosition( halfW - 96, h015 + 52 );
    m_btnNextJet.setPosition( viewportWidth() - 240, halfH - 24 );
    m_btnPrevJet.setPosition( 48, halfH - 24 );

    m_btnWeap1.setPosition( halfW - 196 - 96, h015 + 52 - 76 );
    m_btnWeap2.setPosition( halfW - 96, h015 + 52 - 76 );
    m_btnWeap3.setPosition( halfW + 100, h015 + 52 - 76 );
}

void Game::initRoadAdditions()
{
    m_fontPauseTxt = new Font( "misc/DejaVuSans-Bold.ttf", 18 );
    m_fontGuiTxt = new Font( "misc/DejaVuSans-Bold.ttf", 12 );
    m_fontBig = new Font( "misc/DejaVuSans-Bold.ttf", 32 );

    m_buttonTexture = loadTexture( m_io->getWait( "textures/button1.tga" ) );

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

    constexpr auto weaponToString = []( int i ) -> std::string_view
    {
        using namespace std::string_view_literals;
        switch ( i ) {
        case 0:
            return "Laser"sv;
        case 1:
            return "Blaster"sv;
        case 2:
            return "Torpedo"sv;
        }
        assert( !"invalid weapon id" );
        return "invalid id"sv;
    };

    m_btnWeap1 = Button( weaponToString( m_weap1 ), m_fontGuiTxt, m_buttonTexture );
    m_btnWeap2 = Button( weaponToString( m_weap2 ), m_fontGuiTxt, m_buttonTexture );
    m_btnWeap3 = Button( weaponToString( m_weap1 ), m_fontGuiTxt, m_buttonTexture );

    m_hudTex = loadTexture( m_io->getWait( "textures/HUDtex.tga" ) );
    m_menuBackground = loadTexture( m_io->getWait( "textures/background.tga" ) );
    m_menuBackgroundOverlay = loadTexture( m_io->getWait( "textures/background-overlay.tga" ) );
    m_starfieldTexture = loadTexture( m_io->getWait( "textures/star_field_transparent.tga" ) );
    m_cyberRingTexture[ 0 ] = loadTexture( m_io->getWait( "textures/cyber_ring1.tga" ) );
    m_cyberRingTexture[ 1 ] = loadTexture( m_io->getWait( "textures/cyber_ring2.tga" ) );
    m_cyberRingTexture[ 2 ] = loadTexture( m_io->getWait( "textures/cyber_ring3.tga" ) );

    m_lblPaused = Label{ "PAUSED", Label::HAlign::eCenter, Label::VAlign::eMiddle, m_fontPauseTxt, {}, color::yellow };

    BulletProto tmpWeapon{};
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

    m_enemyModel = new Model{ "models/a2.objc", loadTexture( "textures/a2.tga" ), m_renderer, 0.45f };
}

void Game::updateCyberRings( const UpdateContext& updateContext )
{
    static constexpr auto warp = []( float f )
    {
        constexpr float pie = (float)M_PI;
        if ( f > pie ) { return f - pie * 2.0f; }
        if ( f < -pie ) { return f + pie * 2.0f; }
        return f;
    };
    m_cyberRingRotation[ 0 ] = warp( m_cyberRingRotation[ 0 ] + 25.0_deg * updateContext.deltaTime );
    m_cyberRingRotation[ 1 ] = warp( m_cyberRingRotation[ 1 ] - 15.0_deg * updateContext.deltaTime );

    static float speed = 35.0_deg;
    if ( m_cyberRingRotation[ 2 ] <= 0.0_deg ) {
        speed = 35.0_deg;
    } else if ( m_cyberRingRotation[ 2 ] > 90.0_deg ) {
        speed = -20.0_deg;
    }
    m_cyberRingRotation[ 2 ] += speed * updateContext.deltaTime;
}

void Game::onRender()
{
    m_renderer->clear();
    RenderContext rctx{};
    rctx.renderer = m_renderer;
    rctx.viewport = { m_viewportWidth, m_viewportHeight };
    rctx.projection = glm::ortho<float>( 0.0f, (float)viewportWidth(), 0.0f, (float)viewportHeight(), -100.0f, 100.0f );

    const auto [view, projection] = getCameraMatrix();
    rctx.camera3d = projection * view;

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
}

void Game::onUpdate( const UpdateContext& updateContext )
{
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
    assert( m_jet );
    Jet::Input jetInput{};
    const Uint8* kbd = SDL_GetKeyboardState( nullptr );
    jetInput.pitch += kbd[ SDL_SCANCODE_W ] ? -1.0f : 0.0f;
    jetInput.pitch += kbd[ SDL_SCANCODE_S ] ?  1.0f : 0.0f;
    jetInput.yaw += kbd[ SDL_SCANCODE_Q ] ?  1.0f : 0.0f;
    jetInput.yaw += kbd[ SDL_SCANCODE_E ] ? -1.0f : 0.0f;
    jetInput.roll += kbd[ SDL_SCANCODE_A ] ?  1.0f : 0.0f;
    jetInput.roll += kbd[ SDL_SCANCODE_D ] ? -1.0f : 0.0f;
    jetInput.speed += kbd[ SDL_SCANCODE_U ] ? -1.0f : 0.0f;
    jetInput.speed += kbd[ SDL_SCANCODE_O ] ?  1.0f : 0.0f;
    jetInput.shoot1 = !!kbd[ SDL_SCANCODE_J ];
    jetInput.shoot2 = !!kbd[ SDL_SCANCODE_K ];
    jetInput.shoot3 = !!kbd[ SDL_SCANCODE_L ];
    m_jet->setInput( jetInput );

    if ( m_jet->status() == Jet::Status::eDead ) {
        changeScreen( Screen::eDead );
    }
    if ( m_enemies.empty() ) {
        changeScreen( Screen::eWin );
    }
    m_currentHudColor = m_jet->health() <= 20 ? color::crimson : color::winScreen;

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
        m_jet->update( updateContext );
        const glm::vec3 jetPosition = m_jet->position();
        const glm::vec3 jetVelocity = m_jet->velocity();
        m_speedAnim += m_jet->speed() * 270.0f * updateContext.deltaTime;
        if ( m_speedAnim >= 360 ) {
            m_speedAnim -= 360;
        }
        assert( m_map );
        m_map->setJetData( jetPosition, jetVelocity );
        m_map->update( updateContext );
    }

    {
        for ( Bullet*& b : m_bullets ) {
            assert( b );
            b->update( updateContext );
            for ( Enemy* e : m_enemies ) {
                assert( e );
                b->processCollision( e );
            }

            if ( b->status() == Bullet::Status::eDead ) {
                std::destroy_at( b );
                m_poolBullets.dealloc( b );
                b = nullptr;
            }
        }
        m_bullets.erase( std::remove( m_bullets.begin(), m_bullets.end(), nullptr ), m_bullets.end() );
    }

    {
        for ( Bullet*& b : m_enemyBullets ) {
            b->update( updateContext );
            b->processCollision( m_jet );
            if ( b->status() == Bullet::Status::eDead ) {
                std::destroy_at( b );
                m_poolBullets.dealloc( b );
                b = nullptr;
            }
        }
        m_enemyBullets.erase( std::remove( m_enemyBullets.begin(), m_enemyBullets.end(), nullptr ), m_enemyBullets.end() );
    }
    {
        const auto [ view, projection ] = getCameraMatrix();
        const UpdateContext next{
            .camera = projection * view,
            .viewport = { m_viewportWidth, m_viewportHeight },
            .deltaTime = updateContext.deltaTime,
        };

        for ( auto& e : m_enemies ) {
            e->update( next );
            if ( e->status() == Enemy::Status::eDead ) {
                m_jet->addScore( e->score(), true );
                m_jet->untarget( e );
                std::destroy_at( e );
                m_poolEnemies.dealloc( e );
                e = nullptr;
                continue;
            }
            if ( e->isWeaponReady() ) {
                m_enemyBullets.push_back( e->weapon( m_poolBullets.alloc() ) );
            }
        }
        m_enemies.erase( std::remove( m_enemies.begin(), m_enemies.end(), nullptr ), m_enemies.end() );
    }
    updateCyberRings( updateContext );
    const SAObject* tgt = m_jet->target();
    if ( tgt ) {
        m_targeting.setPos( tgt->position() );
    } else {
        m_targeting.hide();
    }
}

void Game::addBullet( uint32_t wID )
{
    assert( m_audio );
    assert( m_jet );
    if ( !m_jet->isWeaponReady( wID ) ) {
        return;
    }
    m_jet->takeEnergy( wID );
    Bullet* bullet = m_jet->weapon( wID, m_poolBullets.alloc() );
    m_bullets.push_back( bullet );
    m_shotsDone++;
    switch ( bullet->type() ) {
    case Bullet::Type::eBlaster:
        m_audio->play( m_blaster );
        break;
    case Bullet::Type::eSlug:
        m_audio->play( m_laser );
        break;
    case Bullet::Type::eTorpedo:
        m_audio->play( m_torpedo );
        break;
    }
}

void Game::retarget()
{
    if ( m_enemies.empty() ) {
        return;
    }
    static std::mt19937_64 random{ std::random_device()() };
    assert( m_jet );
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
    for ( Enemy* e : m_enemies ) {
        std::destroy_at( e );
    }

    for ( Bullet* b : m_bullets ) {
        std::destroy_at( b );
    }

    for ( Bullet* b : m_enemyBullets ) {
        std::destroy_at( b );
    }
    m_bullets.clear();
    m_enemyBullets.clear();
    m_enemies.clear();
    m_poolBullets.discardAll();
    m_poolEnemies.discardAll();

    delete m_map;
    m_map = nullptr;

    delete m_jet;
    m_jet = nullptr;
}

void Game::createMapData( const MapProto& mapData, const ModelProto& modelData )
{
    m_shotsDone = 0;
    m_jet = new Jet( modelData, m_renderer );
    m_map = new Map( mapData );
    m_jet->setWeapon( m_weapons[ m_weap1 ], 0 );
    m_jet->setWeapon( m_weapons[ m_weap2 ], 1 );
    m_jet->setWeapon( m_weapons[ m_weap3 ], 2 );

    assert( m_enemies.empty() );
    m_enemies.resize( mapData.enemies );
    for ( Enemy*& it : m_enemies ) {
        it = new ( m_poolEnemies.alloc() ) Enemy( m_enemyModel );
        it->setTarget( m_jet );
        it->setWeapon( m_weapons[ 3 ] );
    }

    for ( MapProto& it : m_mapsContainer ) {
        destroyTexture( it.preview_image );
        it.preview_image = Texture{};
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
        assert( !m_mapsContainer.empty() );
        m_btnNextMap.setEnabled( m_currentMap < m_mapsContainer.size() - 1 );
        m_btnPrevMap.setEnabled( m_currentMap > 0 );
        clearMapData();
        m_currentScreen = scr;
        break;

    case Screen::eCustomize:
        assert( !m_jetsContainer.empty() );
        m_btnNextJet.setEnabled( m_currentJet < m_jetsContainer.size() - 1 );
        m_btnPrevJet.setEnabled( m_currentJet > 0 );
        m_modelRotation = 135.0;
        reloadPreviewModel();
        m_currentScreen = scr;
        break;

    default:
        break;
    }
}

void Game::goFullscreen( bool )
{
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
    m_alphaN = fmodf( m_alphaN + updateContext.deltaTime * 0.1f, 1.0f );
    m_alphaValue = 0.65f + 0.35f * std::cos( lerp( -M_PI, M_PI, m_alphaN ) );
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

uint32_t Game::viewportWidth() const
{
    return m_viewportWidth;
}

uint32_t Game::viewportHeight() const
{
    return m_viewportHeight;
}

float Game::viewportAspect() const
{
    return m_viewportAspect;
}

void Game::setViewportSize( uint32_t w, uint32_t h )
{
    m_viewportWidth = w;
    m_viewportHeight = h;
    m_viewportAspect = (float)w / (float)h;
}

void Game::reloadPreviewModel()
{
    Model* model = new Model(
        m_jetsContainer.at( m_currentJet ).model_file
        , loadTexture( m_jetsContainer.at( m_currentJet ).model_texture.c_str() )
        , m_renderer
    );
    std::swap( model, m_previewModel );
    delete model;
}
