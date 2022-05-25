#include "game.hpp"

#include "colors.hpp"
#include "constants.hpp"
#include "game_action.hpp"
#include "game_callbacks.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"

#include <config/config.hpp>

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
    "textures/cyber_ring1.tga",
    "textures/cyber_ring2.tga",
    "textures/cyber_ring3.tga",
    "textures/atlas_ui.tga",
    "maps.cfg",
    "jets.cfg",
    "ui/mainmenu.ui",
    "ui/missionselect.ui",
    "ui/customize.ui",
    "ui/settings.ui",
    "ui/pause.ui",
    "ui/result.ui",
};

static constexpr const char* chunk1[] = {
    "textures/a2.tga",
    "textures/a3.tga",
    "textures/a4.tga",
    "textures/a5.tga",
    "textures/plasma.tga",
};


static constexpr Sprite c_spritesUi[]{
    /*[ ui::AtlasSprite::eBackground ] =*/ { 84, 0, 8, 8 },
    /*[ ui::AtlasSprite::eArrowLeft ]  =*/ { 0, 0, 24, 48 },
    /*[ ui::AtlasSprite::eArrowRight ] =*/ { 24, 0, 24, 48 },
    /*[ ui::AtlasSprite::eTopLeft ]    =*/ { 48, 0, 8, 8 },
    /*[ ui::AtlasSprite::eTop ]        =*/ { 60, 0, 8, 8 },
    /*[ ui::AtlasSprite::eTopRight ]   =*/ { 72, 0, 8, 8 },
    /*[ ui::AtlasSprite::eLeft ]       =*/ { 48, 12, 8, 8 },
    /*[ ui::AtlasSprite::eMid ]        =*/ { 60, 12, 8, 8 },
    /*[ ui::AtlasSprite::eRight ]      =*/ { 72, 12, 8, 8 },
    /*[ ui::AtlasSprite::eBotLeft ]    =*/ { 48, 24, 8, 8 },
    /*[ ui::AtlasSprite::eBot ]        =*/ { 60, 24, 8, 8 },
    /*[ ui::AtlasSprite::eBotRight ]   =*/ { 72, 24, 8, 8 },
    /*[ ui::AtlasSprite::eBotLeft2 ]   =*/ { 84, 12, 8, 8 },
    /*[ ui::AtlasSprite::eBotRight2 ]   =*/ { 84, 24, 8, 8 },
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


static std::pmr::vector<ModelProto> loadJets( std::span<const char> str )
{
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( str );

    std::pmr::vector<ModelProto> jets;
    jets.reserve( 4 );

    using std::literals::string_view_literals::operator""sv;
    for ( const cfg::Entry& it : entry ) {
        std::string_view name = *it;
        jets.emplace_back( ModelProto{
            .name = std::pmr::u32string{ name.begin(), name.end() },
            .model_file = std::string{ it[ "model"sv ].toString() },
            .model_texture = std::string{ it[ "texture"sv ].toString() },
            .scale = it[ "scale"sv ].toFloat(),
        } );
    }

    return jets;
}

static std::pmr::vector<MapCreateInfo> loadMaps( std::span<const char> str )
{
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( str );

    std::pmr::vector<MapCreateInfo> levels;
    levels.reserve( 5 );

    using std::literals::string_view_literals::operator""sv;
    for ( const cfg::Entry& it : entry ) {
        std::string_view name = *it;
        levels.emplace_back( MapCreateInfo{
            .name = std::pmr::u32string{ name.begin(), name.end() },
            .previewPath = it[ "preview"sv ].toString(),
            .enemies = static_cast<uint32_t>( it[ "enemies"sv ].toInt() ),
        } );
        levels.back().filePath[ MapCreateInfo::eTop ] = it[ "top"sv ].toString();
        levels.back().filePath[ MapCreateInfo::eBottom ] = it[ "bottom"sv ].toString();
        levels.back().filePath[ MapCreateInfo::eLeft ] = it[ "left"sv ].toString();
        levels.back().filePath[ MapCreateInfo::eRight ] = it[ "right"sv ].toString();
        levels.back().filePath[ MapCreateInfo::eFront ] = it[ "front"sv ].toString();
        levels.back().filePath[ MapCreateInfo::eBack ] = it[ "back"sv ].toString();
    }

    return levels;
}



Game::Game( int argc, char** argv )
: Engine{ argc, argv }
, m_atlasUi{ c_spritesUi, 96, 48 }
{
    ZoneScoped;
    preloadData();
    for ( const auto& p : g_pipelines ) {
        m_renderer->createPipeline( p );
    }

    changeScreen( Screen::eMainMenu );

    m_enemies.reserve( 100 );
    m_explosions.reserve( 100 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
    m_jetsContainer.reserve( 5 );
    m_mapsContainer.reserve( 5 );
}

Game::~Game()
{
    ZoneScoped;
    clearMapData();

    delete m_fontSmall;
    delete m_fontMedium;
    delete m_fontLarge;

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

void Game::onExit()
{
}

void Game::onResize( uint32_t w, uint32_t h )
{
    ZoneScoped;
    m_glow.setSize( { w, h } );
    m_screenTitle.resize( { w, h } );
    m_screenCustomize.resize( { w, h } );
    m_screenSettings.resize( { w, h } );
    m_screenPause.resize( { w, h } );
    m_screenMissionSelect.resize( { w, h } );
    m_screenMissionResult.resize( { w, h } );
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
    m_displayModes = displayModes();

    m_dustUi.setVelocity(  math::vec3{ 0.0f, 0.0f, 26.0_m }  );
    m_dustUi.setCenter( {} );
    m_dustUi.setLineWidth( 2.0f );

    g_gameCallbacks[ "$function:goto_missionBriefing" ] = [this](){ changeScreen( Screen::eGameBriefing, m_click ); };
    g_gameCallbacks[ "$function:goto_newgame" ] = [this](){ changeScreen( Screen::eMissionSelection, m_click ); };
    g_gameCallbacks[ "$function:goto_customize" ] = [this](){ changeScreen( Screen::eCustomize, m_click ); };
    g_gameCallbacks[ "$function:goto_settings" ] = [this](){ changeScreen( Screen::eSettings, m_click ); };
    g_gameCallbacks[ "$function:goto_titlemenu" ] = [this](){ changeScreen( Screen::eMainMenu, m_click ); };
    g_gameCallbacks[ "$function:quit" ] = [this](){ quit(); };
    g_gameCallbacks[ "$function:resume" ] = [this]{ changeScreen( Screen::eGame, m_click ); };


    g_uiProperty.m_colorA = color::dodgerBlue;

    m_dataModelVSync.m_size = [](){ return 3; };
    m_dataModelVSync.m_at = []( auto i ) -> std::pmr::u32string
    {
        assert( i < 3 );
        using std::literals::string_view_literals::operator""sv;
        static constexpr std::u32string_view s[] = { U"OFF"sv, U"ON"sv, U"MAILBOX"sv };
        return { s[ i ].begin(), s[ i ].end() };
    };
    m_dataModelVSync.m_select = [this]( auto i )
    {
        assert( i < 3 );
        static constexpr VSync v[] = { VSync::eOff, VSync::eOn, VSync::eMailbox };
        m_renderer->setVSync( v[ i ] );
    };
    g_gameUiDataModels[ "$data:vsync" ] = &m_dataModelVSync;

    m_dataModelResolution.m_size = [this]() { return m_displayModes.size(); };
    m_dataModelResolution.m_at = [this]( auto i )
    {
        assert( i < m_displayModes.size() );
        auto d = m_displayModes[ i ];
        return intToUTF32( d.width ) + U" x "
            + intToUTF32( d.height ) + U" @ "
            + intToUTF32( d.rate ) + U"Hz";
    };
    m_dataModelResolution.m_current = [this](){ return m_currentResolution; };
    m_dataModelResolution.m_select = [this]( auto i ) { m_currentResolution = i; };
    m_dataModelResolution.m_activate = [this]( auto i )
    {
        assert( i < m_displayModes.size() );
        setDisplayMode( m_displayModes[ i ], 0 );
    };
    g_gameUiDataModels[ "$data:resolution" ] = &m_dataModelResolution;

    m_dataJet.m_size = [this](){ return m_jetsContainer.size(); };
    m_dataJet.m_at = [this]( auto i )
    {
        assert( i < m_jetsContainer.size() );
        return m_jetsContainer[ i ].name;
    };
    m_dataJet.m_select = [this]( auto i )
    {
        assert( i < m_jetsContainer.size() );
        m_currentJet = i;
    };
    g_gameUiDataModels[ "$data:jet" ] = &m_dataJet;

    auto weapNames = []( auto i ) -> std::pmr::u32string
    {
        assert( i < 3 );
        using std::literals::string_view_literals::operator""sv;
        static constexpr std::u32string_view s[] = { U"Laser"sv, U"Blaster"sv, U"Torpedo"sv };
        return { s[ i ].begin(), s[ i ].end() };
    };
    m_dataWeaponPrimary.m_size = [](){ return 3; };
    m_dataWeaponPrimary.m_at = weapNames;
    m_dataWeaponPrimary.m_select = [this]( auto i ){ m_weapon1 = i; };
    m_dataWeaponPrimary.m_current = [this](){ return m_weapon1; };
    m_dataWeaponSecondary.m_size = [](){ return 3; };
    m_dataWeaponSecondary.m_at = weapNames;
    m_dataWeaponSecondary.m_select = [this]( auto i ){ m_weapon2 = i; };
    m_dataWeaponSecondary.m_current = [this](){ return m_weapon2; };
    g_gameUiDataModels[ "$data:weaponPrimary" ] = &m_dataWeaponPrimary;
    g_gameUiDataModels[ "$data:weaponSecondary" ] = &m_dataWeaponSecondary;


    m_dataMissionSelect.m_current = [this](){ return m_currentMission; };
    m_dataMissionSelect.m_size = [this](){ return m_mapsContainer.size(); };
    m_dataMissionSelect.m_at = [this]( auto i ) -> std::pmr::u32string
    {
        assert( i < m_mapsContainer.size() );
        return m_mapsContainer[ i ].name;
    };
    m_dataMissionSelect.m_select = [this]( auto i )
    {
        assert( i < m_mapsContainer.size() );
        m_currentMission = i;
    };
    m_dataMissionSelect.m_texture = [this]( auto i ) -> Texture
    {
        assert( i < m_mapsContainer.size() );
        return m_mapsContainer[ i ].preview;
    };
    g_gameUiDataModels[ "$data:missionSelect" ] = &m_dataMissionSelect;

    {
        std::pmr::u32string charset = U"0123456789"
        U"abcdefghijklmnopqrstuvwxyz"
        U"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        U" `~'\",./\\?+-*!@#$%^&()[]{};:<>";
        std::sort( charset.begin(), charset.end() );
        const std::pmr::vector<uint8_t> fontFileContent = m_io->getWait( "misc/DejaVuSans-Bold.ttf" );
        const Font::CreateInfo createInfo{
            .fontFileContent = { fontFileContent.cbegin(), fontFileContent.cend() },
            .renderer = m_renderer,
            .charset = charset,
        };
        m_fontSmall = new Font( createInfo, 12 );
        m_fontMedium = new Font( createInfo, 18 );
        m_fontLarge = new Font( createInfo, 32 );
        g_uiProperty.m_fontSmall = m_fontSmall;
        g_uiProperty.m_fontMedium = m_fontMedium;
        g_uiProperty.m_fontLarge = m_fontLarge;
    }
    m_hud = Hud{ &m_hudData };

    m_laser = m_audio->load( "sounds/laser.wav" );
    m_blaster = m_audio->load( "sounds/blaster.wav" );
    m_torpedo = m_audio->load( "sounds/torpedo.wav" );
    m_click = m_audio->load( "sounds/click.wav" );

    const std::array rings = {
        parseTexture( m_io->getWait( "textures/cyber_ring1.tga" ) ),
        parseTexture( m_io->getWait( "textures/cyber_ring2.tga" ) ),
        parseTexture( m_io->getWait( "textures/cyber_ring3.tga" ) ),
    };
    m_uiRings = UIRings{ rings };

    m_textures[ "textures/atlas_ui.tga" ] = parseTexture( m_io->getWait( "textures/atlas_ui.tga" ) );
    g_uiProperty.m_atlasTexture = m_textures[ "textures/atlas_ui.tga" ];
    g_uiProperty.m_atlas = &m_atlasUi;


    for ( const char* it : chunk1 ) {
        m_textures[ it ] = parseTexture( m_io->getWait( it ) );
    }

    m_plasma = m_textures[ "textures/plasma.tga" ];
    BulletProto tmpWeapon{};
    tmpWeapon.texture = m_textures[ "textures/plasma.tga" ];
    tmpWeapon.type = Bullet::Type::eSlug;
    tmpWeapon.delay = 0.05f;
    tmpWeapon.energy = 15;
    tmpWeapon.damage = 1;
    tmpWeapon.score_per_hit = 1;
    tmpWeapon.color1 = color::yellow;
    tmpWeapon.color2 = color::yellow;
    m_weapons[ 0 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eBlaster;
    tmpWeapon.speed = 5100_kmph;
    tmpWeapon.damage = 10;
    tmpWeapon.energy = 10;
    tmpWeapon.delay = 0.1f;
    tmpWeapon.color1 = color::blaster;
    tmpWeapon.color2 = color::dodgerBlue;
    tmpWeapon.score_per_hit = 30;
    m_weapons[ 1 ] = tmpWeapon;

    tmpWeapon.color1 = color::yellowBlaster;
    tmpWeapon.color2 = color::white;
    tmpWeapon.delay = 0.4f;
    m_weapons[ 3 ] = tmpWeapon;

    tmpWeapon.type = Bullet::Type::eTorpedo;
    tmpWeapon.damage = 1;
    tmpWeapon.delay = 0.2f;
    tmpWeapon.energy = 1;
    tmpWeapon.speed = 2400_kmph;
    tmpWeapon.score_per_hit = 2;
    tmpWeapon.color1 = color::orchid;
    tmpWeapon.color2 = color::white;
    m_weapons[ 2 ] = tmpWeapon;

    loadMapProto();

    {
        std::pmr::vector<uint8_t> jetscfg = m_io->getWait( "jets.cfg" );
        const char* ptr = reinterpret_cast<const char*>( jetscfg.data() );
        std::span<const char> str{ ptr, ptr + jetscfg.size() };
        m_jetsContainer = loadJets( str );
        assert( !m_jetsContainer.empty() );
        for ( auto& it : m_jetsContainer ) {
            m_io->enqueue( it.model_file );
        }
        for ( auto& it : m_jetsContainer ) {
            m_meshes[ it.model_file ] = Mesh{ m_io->getWait( it.model_file ), m_renderer };
            it.model = Model( m_meshes[ it.model_file ], m_textures[ it.model_texture ], it.scale );
        }
    }

    auto makeScreen = []( std::string_view strv, auto* io )
    {
        auto ui = io->getWait( strv );
        const char* txt = reinterpret_cast<const char*>( ui.data() );
        std::span<const char> span{ txt, txt + ui.size() };
        return ui::Screen{ cfg::Entry::fromData( span ) };
    };

    m_screenTitle = makeScreen( "ui/mainmenu.ui", m_io );
    m_screenMissionSelect = makeScreen( "ui/missionselect.ui", m_io );
    m_screenCustomize = makeScreen( "ui/customize.ui", m_io );
    m_screenSettings = makeScreen( "ui/settings.ui", m_io );
    m_screenPause = makeScreen( "ui/pause.ui", m_io );
    m_screenMissionResult = makeScreen( "ui/result.ui", m_io );

    m_enemyModel = Model{ m_meshes[ "models/a2.objc" ], m_textures[ "textures/a2.tga" ], 0.45f };

    onResize( viewportWidth(), viewportHeight() );
}

void Game::onRender( RenderContext rctx )
{
    ZoneScoped;
    const auto [ width, height, aspect ] = viewport();
    const auto [ view, projection ] = getCameraMatrix();
    rctx.projection = math::ortho( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ), -100.0f, 100.0f );
    rctx.camera3d = projection * view;
    rctx.viewport = { width, height };

    ui::RenderContext r{
        .renderer = rctx.renderer,
        .model = rctx.model,
        .view = rctx.view,
        .projection = rctx.projection,
        .colorMain = color::dodgerBlue,
        .colorFocus = color::lightSkyBlue,
    };

    switch ( m_currentScreen ) {
    case Screen::eGame:
        renderGameScreen( rctx, r );
        break;

    case Screen::eGamePaused:
        renderGameScreen( rctx, r );
        m_uiRings.render( r );
        m_glow.render( r );
        m_screenPause.render( r );
        break;

    case Screen::eDead:
    case Screen::eWin:
        renderGameScreen( rctx, r );
        m_uiRings.render( r );
        m_glow.render( r );
        m_screenMissionResult.render( r );
        break;

    case Screen::eMissionSelection:
        renderMenuScreen( rctx, r );
        m_screenMissionSelect.render( r );
        break;

    case Screen::eMainMenu:
        renderMenuScreen( rctx, r );
        m_screenTitle.render( r );
        break;

    case Screen::eSettings:
        renderMenuScreen( rctx, r );
        m_screenSettings.render( r );
        break;

    case Screen::eCustomize:
        renderMenuScreen( rctx, r );
        m_screenCustomize.render( r );
        break;

    default:
        break;
    }

    const PushConstant<Pipeline::eGammaCorrection> pushConstant{
        .m_power = 1.2f,
    };

    const DispatchInfo dispatchInfo{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eGammaCorrection ),
    };
    m_renderer->dispatch( dispatchInfo, &pushConstant );
}

void Game::onUpdate( const UpdateContext& uctx )
{
    ZoneScoped;

    switch ( m_currentScreen ) {
    case Screen::eGame:
        updateGame( uctx );
        return;
    case Screen::eGamePaused:
        m_uiRings.update( uctx );
        m_screenPause.update( uctx );
        return;
    case Screen::eDead:
    case Screen::eWin:
        m_uiRings.update( uctx );
        m_screenMissionResult.update( uctx );
        return;

    case Screen::eMissionSelection:
        m_screenMissionSelect.update( uctx );
        break;
    case Screen::eMainMenu:
        m_screenTitle.update( uctx );
        break;
    case Screen::eSettings:
        m_screenSettings.update( uctx );
        break;
    case Screen::eCustomize:
        m_screenCustomize.update( uctx );
        break;
    default:
        break;
    }
    m_dustUi.update( uctx );
    m_uiRings.update( uctx );
}

void Game::pause()
{
    changeScreen( Screen::eGamePaused );
}

void Game::unpause()
{
    changeScreen( Screen::eGame );
}

void Game::updateGame( const UpdateContext& updateContext )
{
    ZoneScoped;

    m_jet.setInput( m_jetInput );

    if ( m_jet.status() == Jet::Status::eDead ) {
        changeScreen( Screen::eDead );
    }
    if ( m_enemies.empty() ) {
        changeScreen( Screen::eWin );
    }

    for ( auto i : { 0u, 1u, 2u } ) {
        if ( m_jet.isShooting( i ) ) {
            addBullet( i );
        }
    }

    m_jet.update( updateContext );
    m_dustGame.setCenter( m_jet.position() );
    m_dustGame.setVelocity( m_jet.velocity() * -0.5f );
    m_dustGame.update( updateContext );

    for ( Explosion& it : m_explosions ) {
        it.update( updateContext );
    }
    m_explosions.erase( std::remove_if( m_explosions.begin(), m_explosions.end(), &Explosion::isInvalid ), m_explosions.end() );

    auto isDead = []( const auto& it ) -> bool { return it->status() == SAObject::Status::eDead; };
    {
        for ( auto& b : m_bullets ) {
            assert( b );
            b->update( updateContext );
            if ( isDead( b ) ) { continue; }
            for ( auto& e : m_enemies ) {
                assert( e );
                if ( !intersectLineSphere( b->position(), b->prevPosition(), e->position(), 15.0_m ) ) {
                    continue;
                }
                e->setDamage( b->damage() );
                m_hudData.score += b->score();
                b->kill();
                break;
            }
        }
        m_bullets.erase( std::remove_if( m_bullets.begin(), m_bullets.end(), isDead ), m_bullets.end() );
    }

    {
        const math::vec3 jetPos = m_jet.position();
        for ( auto& b : m_enemyBullets ) {
            b->update( updateContext );
            if ( isDead( b ) ) { continue; }
            if ( intersectLineSphere( b->position(), b->prevPosition(), jetPos, 15.0_m ) ) {
                m_jet.setDamage( b->damage() );
                b->kill();
            }
        }
        m_enemyBullets.erase( std::remove_if( m_enemyBullets.begin(), m_enemyBullets.end(), isDead ), m_enemyBullets.end() );
    }
    {
        for ( auto& e : m_enemies ) {
            e->update( updateContext );
            if ( isDead( e ) ) {
                m_jet.untarget( *e );
                m_explosions.push_back( Explosion{ e->position(), e->velocity(), m_plasma, 0.0f } );
                continue;
            }
            if ( e->isWeaponReady() ) {
                m_enemyBullets.push_back( e->weapon( &m_poolBullets ) );
            }
        }
        m_enemies.erase( std::remove_if( m_enemies.begin(), m_enemies.end(), isDead ), m_enemies.end() );
    }
    const SAObject* tgt = m_jet.target();
    if ( tgt ) {
        m_targeting.setPos( tgt->position() );
    } else {
        m_targeting.hide();
    }

    m_hudData.calc = static_cast<uint32_t>( m_fpsMeter.calculated() );
    m_hudData.fps = static_cast<uint32_t>( m_fpsMeter.fps() );
    m_hudData.pool = static_cast<uint32_t>( m_poolBullets.allocCount() );
    m_hudData.speed = m_jet.speed();
    m_hudData.hp = static_cast<float>( m_jet.health() ) / 100.0f;
    m_hudData.pwr = static_cast<float>( m_jet.energy() ) / 100.0f;
    m_hud.update( updateContext );
}

void Game::addBullet( uint32_t wID )
{
    ZoneScoped;
    assert( m_audio );
    if ( !m_jet.isWeaponReady( wID ) ) {
        return;
    }
    m_jet.takeEnergy( wID );
    const auto& bullet = m_bullets.emplace_back( m_jet.weapon( wID, &m_poolBullets ) );

    m_hudData.shots++;
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
    m_jet.setTarget( *m_enemies[ random() % m_enemies.size() ] );
}

void Game::clearMapData()
{
    ZoneScoped;
    m_enemies.clear();
    m_bullets.clear();
    m_enemyBullets.clear();
}

void Game::createMapData( const MapCreateInfo& mapInfo, const ModelProto& modelData )
{
    ZoneScoped;
    m_hudData = HudData{
        .hp = 1.0f,
        .pwr = 1.0f,
    };
    m_skybox = Skybox{ mapInfo.texture };
    m_jet = Jet( modelData );
    m_jet.setWeapon( m_weapons[ m_weapon1 ], 0 );
    m_jet.setWeapon( m_weapons[ m_weapon2 ], 1 );
    m_jet.setWeapon( m_weapons[ m_weapon1 ], 2 );

    assert( m_enemies.empty() );
    m_enemies.resize( mapInfo.enemies );
    std::generate( m_enemies.begin(), m_enemies.end(), [this]()
    {
        UniquePointer<Enemy> ptr{ &m_poolEnemies, &m_enemyModel };
        ptr->setTarget( &m_jet );
        ptr->setWeapon( m_weapons[ 3 ] );
        return ptr;
    });

}

void Game::changeScreen( Screen scr, Audio::Slot sound )
{
    ZoneScoped;
    if ( sound ) {
        m_audio->play( sound );
    }
    SDL_ShowCursor( scr != Screen::eGame );

    m_screenCustomize.show();
    m_screenMissionResult.show();
    m_screenMissionSelect.show();
    m_screenPause.show();
    m_screenSettings.show();
    m_screenTitle.show();

    switch ( scr ) {
    case Screen::eMainMenu:
    case Screen::eSettings:
    case Screen::eCustomize:
        m_currentScreen = scr;
        break;

    case Screen::eGame:
    case Screen::eGamePaused:
        m_dustGame.setVelocity( m_jet.velocity() * -0.5f );
        m_dustGame.setCenter( m_jet.position() );
        m_dustGame.setLineWidth( 1.618f );
        m_currentScreen = scr;
        break;

    case Screen::eDead:
        m_uiMissionResult = U"Mission Failed";
        m_uiScore = m_hudData.score;
        m_currentScreen = scr;
        break;

    case Screen::eWin:
        m_uiMissionResult = U"Mission Successful";
        m_uiScore = m_hudData.score;
        m_currentScreen = scr;
        break;

    case Screen::eGameBriefing:
        createMapData( m_mapsContainer[ m_currentMission ], m_jetsContainer[ m_currentJet ] );
        changeScreen( Screen::eGamePaused );
        break;

    case Screen::eMissionSelection:
        clearMapData();
        m_currentScreen = scr;
        break;

    default:
        assert( !"unhandled enum" );
        break;
    }
}

void Game::loadMapProto()
{
    ZoneScoped;
    assert( m_mapsContainer.empty() );

    std::pmr::vector<uint8_t> mapscfg = m_io->getWait( "maps.cfg" );
    const char* ptr = reinterpret_cast<const char*>( mapscfg.data() );
    std::span<const char> str{ ptr, ptr + mapscfg.size() };
    m_mapsContainer = loadMaps( str );
    assert( !m_mapsContainer.empty() );

    std::pmr::set<std::filesystem::path> uniqueTextures;
    for ( auto it : m_mapsContainer ) {
        uniqueTextures.insert( it.previewPath );
        for ( auto p : it.filePath ) {
            uniqueTextures.insert( p );
        }
    }

    for ( const auto& it : uniqueTextures ) {
        m_io->enqueue( it );
    }
    for ( const auto& it : uniqueTextures ) {
        m_textures[ it ] = parseTexture( m_io->getWait( it ) );
    }
    for ( auto& it : m_mapsContainer ) {
        for ( size_t i = 0; i < it.texture.size(); ++i ) {
            it.texture[ i ] = m_textures[ it.filePath[ i ] ];
            assert( it.texture[ i ] );
        }
        it.preview = m_textures[ it.previewPath ];
        assert( it.preview );
    }

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
    ZoneScoped;
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

    case Screen::eSettings:
        m_screenSettings.onAction( a );
        return;

    case Screen::eGamePaused:
        m_screenPause.onAction( a );
        return;

    case Screen::eWin:
    case Screen::eDead:
        m_screenMissionResult.onAction( a );
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
        assert( !"unhandled enum" );
        return;
    }
}

void Game::onMouseEvent( const MouseEvent& mouseEvent )
{
    ZoneScoped;
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
    case Screen::eWin:
        m_screenMissionResult.onMouseEvent( mouseEvent );
        break;

    case Screen::eCustomize:
        m_screenCustomize.onMouseEvent( mouseEvent );
        break;

    case Screen::eSettings:
        m_screenSettings.onMouseEvent( mouseEvent );
        break;

    case Screen::eGame:
        break;

    default:
        assert( !"unhandled enum" );
        break;
    }
}

void Game::renderGameScreen( RenderContext rctx, ui::RenderContext r )
{
    render3D( rctx );
    m_hud.render( r );
    m_targeting.render( rctx );
}

std::tuple<math::mat4, math::mat4> Game::getCameraMatrix() const
{
    const math::vec3 cameraPos = m_jet.position() + math::vec3{ 0, -10.5_m, 41.5_m } * m_jet.rotation();
    const math::vec3 cameraUp = math::vec3{ 0, 1, 0 } * m_jet.rotation();
    const math::vec3 cameraTgt = cameraPos + m_jet.direction();
    return {
        math::lookAt( cameraPos, cameraTgt, cameraUp ),
        math::perspective( math::radians( 55.0f + m_jet.speed() * 3 ), viewportAspect(), 0.001f, 2000.0f )
    };
}

void Game::render3D( RenderContext rctx )
{
    std::tie( rctx.view, rctx.projection ) = getCameraMatrix();

    rctx.cameraPosition = m_jet.position() + math::vec3{ 0, -10.5_m, 41.5_m } * m_jet.rotation();
    rctx.cameraUp = math::vec3{ 0, 1, 0 } * m_jet.rotation();

    m_skybox.render( rctx );
    for ( const Explosion& it : m_explosions ) {
        it.render( rctx );
    }
    m_dustGame.render( rctx );
    for ( auto& it : m_enemies ) {
        assert( it );
        it->render( rctx );
    }

    for ( auto& it : m_bullets ) {
        assert( it );
        it->render( rctx );
    }
    for ( auto& it : m_enemyBullets ) {
        assert( it );
        it->render( rctx );
    }
    m_jet.render( rctx );
}

void Game::renderBackground( ui::RenderContext rctx ) const
{
    [[maybe_unused]]
    const auto [ w, h, a ] = viewport();
    const math::vec2 uv = math::vec2{ w, h } / m_atlasUi.extent();

    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eBackground ),
        .m_verticeCount = 4,
        .m_texture = g_uiProperty.atlasTexture(),
    };
    PushConstant<Pipeline::eBackground> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = rctx.colorMain,
        .m_uvSlice = m_atlasUi.sliceUV( ui::AtlasSprite::eBackground ),
        .m_xyuv{
            math::vec4{ 0, 0, 0, 0 },
            math::vec4{ 0, h, 0, uv.y },
            math::vec4{ w, h, uv.x, uv.y },
            math::vec4{ w, 0, uv.x, 0 }
        },
    };
    rctx.renderer->push( pushBuffer, &pushConstant );
}

void Game::renderMenuScreen( RenderContext rctx, ui::RenderContext r ) const
{
    renderBackground( r );
    rctx.projection = math::perspective(
        55.0_deg
        , viewportAspect()
        , 0.001f
        , 2000.0f
    );
    const math::vec3 cameraPos = math::normalize( math::vec3{ -4, -3, -3 } ) * 24.0_m;
    rctx.view = math::lookAt( cameraPos, math::vec3{}, math::vec3{ 0, 1, 0 } );

    auto& jet = m_jetsContainer[ m_currentJet ].model;
    jet.render( rctx );
    m_dustUi.render( rctx );

    m_uiRings.render( r );
    m_glow.render( r );
}

