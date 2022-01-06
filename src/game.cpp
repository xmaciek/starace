#include "game.hpp"

#include "colors.hpp"
#include "constants.hpp"
#include "game_action.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"

#include <Tracy.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <cmath>
#include <set>

static constexpr const char* chunk0[] = {
    "misc/DejaVuSans-Bold.ttf",
    "textures/button1.tga",
    "textures/HUDtex.tga",
    "textures/background.tga",
    "textures/background-overlay.tga",
    "textures/star_field_transparent.tga",
    "textures/cyber_ring1.tga",
    "textures/cyber_ring2.tga",
    "textures/cyber_ring3.tga",
};

static constexpr const char* chunk1[] = {
    "textures/a2.tga",
    "textures/a3.tga",
    "textures/a4.tga",
    "textures/a5.tga",
};

constexpr std::tuple<GameAction, Actuator> inputActions[] = {
    { GameAction::eGamePause, SDL_CONTROLLER_BUTTON_START },
    { GameAction::eGamePause, SDL_SCANCODE_ESCAPE },
    { GameAction::eJetPitch, SDL_CONTROLLER_AXIS_LEFTY },
    { GameAction::eJetRoll, SDL_CONTROLLER_AXIS_RIGHTX },
    { GameAction::eJetShoot1, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { GameAction::eJetShoot1, SDL_SCANCODE_J },
    { GameAction::eJetShoot2, SDL_CONTROLLER_BUTTON_LEFTSHOULDER },
    { GameAction::eJetShoot2, SDL_SCANCODE_K },
    { GameAction::eJetShoot3, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { GameAction::eJetShoot3, SDL_SCANCODE_L },
    { GameAction::eJetTarget, SDL_CONTROLLER_BUTTON_Y },
    { GameAction::eJetTarget, SDL_SCANCODE_I },
    { GameAction::eJetYaw, SDL_CONTROLLER_AXIS_LEFTX },
    { GameAction::eMenuCancel, SDL_CONTROLLER_BUTTON_B },
    { GameAction::eMenuCancel, SDL_SCANCODE_ESCAPE },
    { GameAction::eMenuConfirm, SDL_CONTROLLER_BUTTON_A },
    { GameAction::eMenuConfirm, SDL_SCANCODE_RETURN },
    { GameAction::eMenuDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN },
    { GameAction::eMenuDown, SDL_SCANCODE_DOWN },
    { GameAction::eMenuDown, SDL_SCANCODE_S },
    { GameAction::eMenuLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT },
    { GameAction::eMenuLeft, SDL_SCANCODE_A },
    { GameAction::eMenuLeft, SDL_SCANCODE_LEFT },
    { GameAction::eMenuRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT },
    { GameAction::eMenuRight, SDL_SCANCODE_D },
    { GameAction::eMenuRight, SDL_SCANCODE_RIGHT },
    { GameAction::eMenuUp, SDL_CONTROLLER_BUTTON_DPAD_UP },
    { GameAction::eMenuUp, SDL_SCANCODE_UP },
    { GameAction::eMenuUp, SDL_SCANCODE_W },
};

constexpr std::tuple<GameAction, Actuator, Actuator> inputActions2[] = {
    { GameAction::eJetPitch, SDL_SCANCODE_W, SDL_SCANCODE_S },
    { GameAction::eJetYaw, SDL_SCANCODE_Q, SDL_SCANCODE_E },
    { GameAction::eJetRoll, SDL_SCANCODE_A, SDL_SCANCODE_D },
    { GameAction::eJetSpeed, SDL_SCANCODE_U, SDL_SCANCODE_O },
    { GameAction::eJetSpeed, SDL_CONTROLLER_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT },
};

Game::Game( int argc, char** argv )
: Engine{ argc, argv }
{
    ZoneScoped;
    m_renderer->createPipeline( g_pipelineGui );
    m_renderer->createPipeline( g_pipelineLineStripColor );
    m_renderer->createPipeline( g_pipelineTriangleFan3DTexture );
    m_renderer->createPipeline( g_pipelineTriangleFan3DColor );
    m_renderer->createPipeline( g_pipelineLine3DColor );
    m_renderer->createPipeline( g_pipelineShortString );
    m_renderer->createPipeline( g_pipelineTriangle3DTextureNormal );
    m_renderer->createPipeline( g_pipelineProgressBar );

    preloadData();
    changeScreen( Screen::eMainMenu );

    m_enemies.reserve( 100 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
}

Game::~Game()
{
    ZoneScoped;
    clearMapData();

    delete m_fontPauseTxt;
    delete m_fontGuiTxt;
    delete m_fontBig;

    delete m_enemyModel;
}

void Game::preloadData()
{
    ZoneScoped;
    for ( const char* it : chunk0 ) {
        m_io->enqueue( it );
    }

    for ( const char* it : chunk1 ) {
        m_io->enqueue( it );
    }
}

void Game::onEvent( const SDL_Event& event )
{
    switch ( event.type ) {
    case SDL_WINDOWEVENT:
        switch ( event.window.event ) {
        case SDL_WINDOWEVENT_RESIZED:
            setViewport( event.window.data1, event.window.data2 );
            onResize( viewportWidth(), viewportHeight() );
        default:
            break;
        }
        break;
    }
}

void Game::onExit()
{
    for ( auto& it : m_jetsContainer ) {
        delete it.model;
    }
}

void Game::onResize( uint32_t w, uint32_t h )
{
    m_maxDimention = std::max( w, h );

    m_screenTitle.resize( { w, h } );
    m_screenCustomize.resize( { w, h } );
    m_screenPause.resize( { w, h } );
    m_screenMissionSelect.resize( { w, h } );
    m_screenWin.resize( { w, h } );
    m_screenLoose.resize( { w, h } );
    m_hud.resize( { w, h } );
    m_uiRings.setSize( { w, h } );
}

void Game::onInit()
{
    ZoneScoped;
    for ( auto [ eid, act ] : inputActions ) {
        registerAction( static_cast<Action::Enum>( eid ), act );
    }
    for ( auto [ eid, min, max ] : inputActions2 ) {
        registerAction( static_cast<Action::Enum>( eid ), min, max );
    }

    m_laser = m_audio->load( "sounds/laser.wav" );
    m_blaster = m_audio->load( "sounds/blaster.wav" );
    m_torpedo = m_audio->load( "sounds/torpedo.wav" );
    m_click = m_audio->load( "sounds/click.wav" );

    {
        std::pmr::u32string charset = U"0123456789"
        U"abcdefghijklmnopqrstuvwxyz"
        U"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        U" `~'\",./\\?+-*!@#$%^&()[]{};:<>";
        std::sort( charset.begin(), charset.end() );
        const std::pmr::vector<uint8_t> fontFileContent = m_io->getWait( "misc/DejaVuSans-Bold.ttf" );
        const Font::CreateInfo createInfo{
            .fontFileContent = &fontFileContent,
            .renderer = m_renderer,
            .charset = charset,
        };

        m_fontPauseTxt = new Font( createInfo, 18 );
        m_fontGuiTxt = new Font( createInfo, 12 );
        m_fontBig = new Font( createInfo, 32 );
    }
    m_hud = Hud{ &m_hudData, m_fontGuiTxt };

    m_buttonTexture = loadTexture( m_io->getWait( "textures/button1.tga" ) );
    m_hudTex = loadTexture( m_io->getWait( "textures/HUDtex.tga" ) );
    m_menuBackground = loadTexture( m_io->getWait( "textures/background.tga" ) );
    m_menuBackgroundOverlay = loadTexture( m_io->getWait( "textures/background-overlay.tga" ) );
    m_starfieldTexture = loadTexture( m_io->getWait( "textures/star_field_transparent.tga" ) );

    const std::array rings = {
        loadTexture( m_io->getWait( "textures/cyber_ring1.tga" ) ),
        loadTexture( m_io->getWait( "textures/cyber_ring2.tga" ) ),
        loadTexture( m_io->getWait( "textures/cyber_ring3.tga" ) ),
    };
    m_uiRings = UIRings{ rings };

    m_screenPause = ScreenPause{
        m_fontGuiTxt,
        m_fontPauseTxt,
        &m_uiRings,
        m_hudTex,
        m_buttonTexture,
        U"PAUSED", [this](){ changeScreen( Screen::eGame ); },
        U"Resume", [this](){ changeScreen( Screen::eGame, m_click ); },
        U"Quit Mission", [this](){ changeScreen( Screen::eMissionSelection, m_click ); }
    };

    m_screenTitle = ScreenTitle{
        m_fontGuiTxt,
        m_buttonTexture,
        U"Select Mission", [this](){ changeScreen( Screen::eMissionSelection, m_click ); },
        U"Customize", [this](){ changeScreen( Screen::eCustomize, m_click ); },
        U"Exit Game", [this](){ quit(); }
    };

    auto onWinLoose = [this]()
    {
        m_audio->play( m_click );
        changeScreen( Screen::eMissionSelection );
    };
    m_screenWin = ScreenWinLoose{
        m_hudTex
        , m_buttonTexture
        , color::winScreen
        , m_fontGuiTxt
        , m_fontBig
        , U"MISSION SUCCESSFULL"
        , decltype( onWinLoose ){ onWinLoose }
    };
    m_screenLoose = ScreenWinLoose{
        m_hudTex
        , m_buttonTexture
        , color::crimson
        , m_fontGuiTxt
        , m_fontBig
        , U"MISSION FAILED"
        , decltype( onWinLoose ){ onWinLoose }
    };


    BulletProto tmpWeapon{};
    tmpWeapon.type = Bullet::Type::eSlug;
    tmpWeapon.delay = 0.05;
    tmpWeapon.energy = 15;
    tmpWeapon.damage = 1;
    tmpWeapon.score_per_hit = 1;
    tmpWeapon.color1 = color::yellow;
    tmpWeapon.color2 = color::yellow;
    m_weapons[ 0 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eBlaster;
    tmpWeapon.speed = 32;
    tmpWeapon.damage = 10;
    tmpWeapon.energy = 10;
    tmpWeapon.delay = 0.1;
    tmpWeapon.color1 = color::blaster;
    tmpWeapon.score_per_hit = 30;
    m_weapons[ 1 ] = tmpWeapon;

    tmpWeapon.color1 = color::yellowBlaster;
    tmpWeapon.delay = 0.4;
    m_weapons[ 3 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eTorpedo;
    tmpWeapon.damage = 1;
    tmpWeapon.delay = 0.2;
    tmpWeapon.energy = 1;
    tmpWeapon.speed = 16;
    tmpWeapon.score_per_hit = 2;
    tmpWeapon.color1 = color::orchid;
    m_weapons[ 2 ] = tmpWeapon;

    for ( const char* it : chunk1 ) {
        m_textures[ it ] = loadTexture( m_io->getWait( it ) );
    }

    loadMapProto();
    std::pmr::vector<MissionInfo> mapInfo{};
    mapInfo.reserve( m_mapsContainer.size() );
    std::transform( m_mapsContainer.begin(), m_mapsContainer.end(), std::back_inserter( mapInfo ),
    []( const MapCreateInfo& mci ) -> MissionInfo {
        return MissionInfo{
            .m_title = std::pmr::u32string{ mci.name.cbegin(), mci.name.cend() }, // TODO utf32 map names
            .m_preview = mci.texture[ MapCreateInfo::ePreview ],
            .m_enemyCount = mci.enemies,
        };
    } );
    m_screenMissionSelect = ScreenMissionSelect{
        std::move( mapInfo )
        , m_fontGuiTxt
        , m_fontPauseTxt
        , m_fontBig
        , &m_uiRings
        , m_hudTex
        , m_buttonTexture
        , U"Enemies:"
        , U"Previous Map", [this](){ if ( m_screenMissionSelect.prev() ) { m_audio->play( m_click ); } }
        , U"Next Map", [this](){ if ( m_screenMissionSelect.next() ) { m_audio->play( m_click ); } }
        , U"Return", [this](){ changeScreen( Screen::eMainMenu, m_click ); }
        , U"Start Mission", [this](){ changeScreen( Screen::eGameBriefing, m_click ); }
    };

    loadJetProto();
    std::pmr::vector<CustomizeData> data{};
    data.reserve( m_jetsContainer.size() );
    std::transform( m_jetsContainer.begin(), m_jetsContainer.end(), std::back_inserter( data ),
        []( ModelProto& it ) { return CustomizeData{ it.name, it.model, 1.0f }; }
    );

    m_screenCustomize = ScreenCustomize{
        { 1, 2, 1 }
        , std::move( data )
        , m_fontGuiTxt
        , m_fontPauseTxt
        , m_hudTex
        , m_buttonTexture
        , &m_uiRings
        , U"Done", [this](){ changeScreen( Screen::eMainMenu, m_click ); }
        , U"Previous Jet", [this](){ if ( m_screenCustomize.prevJet() ) { m_audio->play( m_click ); } }
        , U"Next Jet",  [this](){ if ( m_screenCustomize.nextJet() ) { m_audio->play( m_click ); } }
        , [this](){ m_screenCustomize.nextWeap( 0 ); m_audio->play( m_click ); }
        , [this](){ m_screenCustomize.nextWeap( 1 ); m_audio->play( m_click ); }
        , [this](){ m_screenCustomize.nextWeap( 2 ); m_audio->play( m_click ); }
    };
    m_enemyModel = new Model{ "models/a2.objc", m_textures[ "textures/a2.tga" ], m_renderer, 0.45f };

    onResize( viewportWidth(), viewportHeight() );
}

void Game::onRender( RenderContext rctx )
{
    ZoneScoped;
    const auto [ width, height, aspect ] = viewport();
    const auto [ view, projection ] = getCameraMatrix();
    rctx.projection = glm::ortho<float>( 0.0f, (float)width, 0.0f, (float)height, -100.0f, 100.0f );
    rctx.camera3d = projection * view;
    rctx.viewport = { width, height };

    switch ( m_currentScreen ) {
    case Screen::eGame:
        renderGameScreen( rctx );
        break;
    case Screen::eGamePaused:
        renderGameScreen( rctx );
        m_screenPause.render( rctx );
        break;
        break;
    case Screen::eDead:
        renderDeadScreen( rctx );
        break;
    case Screen::eWin:
        renderWinScreen( rctx );
        break;
    case Screen::eMissionSelection:
        m_screenMissionSelect.render( rctx );
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
    ZoneScoped;
    switch ( m_currentScreen ) {
    case Screen::eGame:
        updateGame( updateContext );
        break;
    case Screen::eMissionSelection:
        m_screenMissionSelect.update( updateContext );
        break;
    case Screen::eGamePaused:
        m_screenPause.update( updateContext );
        break;
    case Screen::eDead:
        updateDeadScreen( updateContext );
        break;
    case Screen::eWin:
        updateWin( updateContext );
        break;
    case Screen::eMainMenu:
        updateMainMenu( updateContext );
        break;
    case Screen::eCustomize:
        m_screenCustomize.update( updateContext );
        break;
    default:
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
    m_uiRings.update( updateContext );
}

void Game::updateGame( const UpdateContext& updateContext )
{
    assert( m_jet );

    m_jet->setInput( m_jetInput );

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
        [[maybe_unused]]
        const auto [ width, height, aspect ] = viewport();
        const auto [ view, projection ] = getCameraMatrix();
        const UpdateContext next{
            .camera = projection * view,
            .viewport = { width, height },
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
    m_uiRings.update( updateContext );
    const SAObject* tgt = m_jet->target();
    if ( tgt ) {
        m_targeting.setPos( tgt->position() );
    } else {
        m_targeting.hide();
    }

    m_hudData.score = m_jet->score();
    m_hudData.shots = m_shotsDone;
    m_hudData.calc = (uint32_t)m_fpsMeter.calculated();
    m_hudData.fps = (uint32_t)m_fpsMeter.fps();
    m_hudData.pool = m_poolBullets.allocCount();
    m_hudData.speed = m_jet->speed();
    m_hudData.hp = static_cast<float>( m_jet->health() ) / 100.0f;
    m_hudData.pwr = static_cast<float>( m_jet->energy() ) / 100.0f;
    m_hud.update( updateContext );
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
    static Random random{ std::random_device()() };
    assert( m_jet );
    m_jet->lockTarget( m_enemies[ random() % m_enemies.size() ] );
}

void Game::updateMainMenu( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
    m_uiRings.update( updateContext );
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

void Game::createMapData( const MapCreateInfo& mapInfo, const ModelProto& modelData )
{
    m_shotsDone = 0;
    m_jet = new Jet( modelData, m_renderer );
    m_map = new Map( mapInfo );
    auto weap = m_screenCustomize.weapons();
    m_jet->setWeapon( m_weapons[ weap[ 0 ] ], 0 );
    m_jet->setWeapon( m_weapons[ weap[ 1 ] ], 1 );
    m_jet->setWeapon( m_weapons[ weap[ 2 ] ], 2 );

    assert( m_enemies.empty() );
    m_enemies.resize( mapInfo.enemies );
    for ( Enemy*& it : m_enemies ) {
        it = new ( m_poolEnemies.alloc() ) Enemy( m_enemyModel );
        it->setTarget( m_jet );
        it->setWeapon( m_weapons[ 3 ] );
    }

}

void Game::updateMissionSelection( const UpdateContext& updateContext )
{
    m_uiRings.update( updateContext );
}

void Game::changeScreen( Screen scr, Audio::Chunk sound )
{
    if ( sound.data ) {
        m_audio->play( sound );
    }
    SDL_ShowCursor( scr != Screen::eGame );

    switch ( scr ) {
    case Screen::eGame:
    case Screen::eGamePaused:
    case Screen::eDead:
    case Screen::eMainMenu:
    case Screen::eWin:
    case Screen::eCustomize:
        m_currentScreen = scr;
        break;

    case Screen::eGameBriefing: {
        const uint32_t currentJet = m_screenCustomize.jet();
        createMapData( m_mapsContainer.at( m_screenMissionSelect.selectedMission() ), m_jetsContainer.at( currentJet ) );
        m_currentScreen = Screen::eGamePaused;
    } break;

    case Screen::eMissionSelection:
        clearMapData();
        m_currentScreen = scr;
        break;

    default:
        break;
    }
}

void Game::loadMapProto()
{
    m_mapsContainer.clear();
    MapCreateInfo map;
    std::ifstream MapFile( "maps.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::pmr::string line;
    std::pmr::set<std::pmr::string> uniqueTextures{};
    while ( getline( MapFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "[MAP]" ) == 0 ) {
            m_mapsContainer.push_back( map );
        }
        else if ( strcmp( value_1, "name" ) == 0 ) {
            m_mapsContainer.back().name = value_2;
        }
        else if ( strcmp( value_1, "enemies" ) == 0 ) {
            m_mapsContainer.back().enemies = atoi( value_2 );
        }
        else if ( strcmp( value_1, "top" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eTop ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "bottom" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eBottom ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "left" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eLeft ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "right" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eRight ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "front" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eFront ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "back" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::eBack ] = value_2;
            uniqueTextures.insert( value_2 );
        }
        else if ( strcmp( value_1, "preview" ) == 0 ) {
            m_mapsContainer.back().filePath[ Map::Wall::ePreview ] = value_2;
            uniqueTextures.insert( value_2 );
        }
    }
    MapFile.close();
    for ( const auto& it : uniqueTextures ) {
        m_io->enqueue( it );
    }
    for ( const auto& it : uniqueTextures ) {
        m_textures[ it ] = loadTexture( m_io->getWait( it ) );
    }
    for ( auto& it : m_mapsContainer ) {
        for ( size_t i = 0; i < it.texture.size(); ++i ) {
            it.texture[ i ] = m_textures[ it.filePath[ i ] ];
            assert( it.texture[ i ] );
        }
    }
}

void Game::loadJetProto()
{
    assert( m_jetsContainer.empty() );
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
            std::string_view v = value_2;
            m_jetsContainer.back().name = std::u32string{ v.begin(), v.end() };
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
    assert( !m_jetsContainer.empty() );

    for ( auto& it : m_jetsContainer ) {
        it.model = new Model( it.model_file, m_textures[ it.model_texture ], m_renderer, 1.0f );
    }
}


void Game::updateClouds( const UpdateContext& updateContext )
{
    m_alphaN = fmodf( m_alphaN + updateContext.deltaTime * 0.1f, 1.0f );
    m_alphaValue = 0.65f + 0.35f * std::cos( std::lerp( -constants::pi, constants::pi, m_alphaN ) );
}

void Game::updateCustomize( const UpdateContext& updateContext )
{
    updateClouds( updateContext );
}

void Game::updateWin( const UpdateContext& updateContext )
{
    m_uiRings.update( updateContext );
}

void Game::updateDeadScreen( const UpdateContext& updateContext )
{
    m_uiRings.update( updateContext );
}

uint32_t Game::viewportWidth() const
{
    return std::get<0>( viewport() );
}

uint32_t Game::viewportHeight() const
{
    return std::get<1>( viewport() );
}

float Game::viewportAspect() const
{
    return std::get<2>( viewport() );
}

void Game::onAction( Action a )
{
    const GameAction action = a.toA<GameAction>();
    switch ( action ) {
    case GameAction::eJetPitch: m_jetInput.pitch = -a.analog; break;
    case GameAction::eJetYaw: m_jetInput.yaw = -a.analog; break;
    case GameAction::eJetRoll: m_jetInput.roll = a.analog; break;
    case GameAction::eJetShoot1: m_jetInput.shoot1 = a.digital; break;
    case GameAction::eJetShoot2: m_jetInput.shoot2 = a.digital; break;
    case GameAction::eJetShoot3: m_jetInput.shoot3 = a.digital; break;
    case GameAction::eJetSpeed: m_jetInput.speed = a.analog; break;
    default: break;
    }

    switch ( m_currentScreen ) {
    case Screen::eMainMenu:
        m_screenTitle.onAction( a );
        return;

    case Screen::eCustomize:
        m_screenCustomize.onAction( a );
        break;

    case Screen::eMissionSelection:
        m_screenMissionSelect.onAction( a );
        return;

    case Screen::eGamePaused:
        m_screenPause.onAction( a );
        return;

    case Screen::eWin:
        m_screenWin.onAction( a );
        return;

    case Screen::eDead:
        m_screenLoose.onAction( a );
        return;

    case Screen::eGame:
        switch ( action ) {
        case GameAction::eJetTarget:
            if ( a.digital ) { retarget(); }
            return;
        case GameAction::eGamePause:
            if ( a.digital ) { pause(); }
            return;
        default:
            return;
        }

    default:
        return;
    }
}

void Game::onMouseEvent( const MouseEvent& mouseEvent )
{
    switch ( m_currentScreen ) {
    case Screen::eGamePaused:
        m_screenPause.onMouseEvent( mouseEvent );
        break;

    case Screen::eMainMenu:
        m_screenTitle.onMouseEvent( mouseEvent );
        break;

    case Screen::eMissionSelection:
        m_screenMissionSelect.onMouseEvent( mouseEvent );
        break;

    case Screen::eDead:
        m_screenLoose.onMouseEvent( mouseEvent );
        break;

    case Screen::eWin:
        m_screenWin.onMouseEvent( mouseEvent );
        break;

    case Screen::eCustomize:
        m_screenCustomize.onMouseEvent( mouseEvent );
        break;

    default:
        break;
    }
}
