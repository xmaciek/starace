#include "game.hpp"

#include "colors.hpp"
#include "game_action.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"
#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <ui/message_box.hpp>

#include <config/config.hpp>

#include <profiler.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <random>
#include <set>
#include <numeric>

constexpr std::tuple<GameAction, input::Actuator> inputActions[] = {
    { GameAction::eGamePause, SDL_CONTROLLER_BUTTON_START },
    { GameAction::eGamePause, SDL_SCANCODE_ESCAPE },
    { GameAction::eJetPitch, SDL_CONTROLLER_AXIS_LEFTY },
    { GameAction::eJetRoll, SDL_CONTROLLER_AXIS_RIGHTX },
    { GameAction::eJetShoot1, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { GameAction::eJetShoot1, SDL_SCANCODE_J },
    { GameAction::eJetShoot2, SDL_CONTROLLER_BUTTON_X },
    { GameAction::eJetShoot2, SDL_SCANCODE_K },
    { GameAction::eJetTarget, SDL_CONTROLLER_BUTTON_Y },
    { GameAction::eJetTarget, SDL_SCANCODE_I },
    { GameAction::eJetLookAt, SDL_CONTROLLER_BUTTON_LEFTSHOULDER },
    { GameAction::eJetLookAt, SDL_SCANCODE_SPACE },
    { GameAction::eJetYaw, SDL_CONTROLLER_AXIS_LEFTX },
};

constexpr std::tuple<ui::Action::Enum, input::Actuator> UI_INPUT[] = {
    { ui::Action::eMenuApply, SDL_CONTROLLER_BUTTON_START },
    { ui::Action::eMenuApply, SDL_SCANCODE_SPACE },
    { ui::Action::eMenuCancel, SDL_CONTROLLER_BUTTON_B },
    { ui::Action::eMenuCancel, SDL_SCANCODE_ESCAPE },
    { ui::Action::eMenuConfirm, SDL_CONTROLLER_BUTTON_A },
    { ui::Action::eMenuConfirm, SDL_SCANCODE_RETURN },
    { ui::Action::eMenuDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN },
    { ui::Action::eMenuDown, SDL_SCANCODE_DOWN },
    { ui::Action::eMenuDown, SDL_SCANCODE_S },
    { ui::Action::eMenuLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT },
    { ui::Action::eMenuLeft, SDL_SCANCODE_A },
    { ui::Action::eMenuLeft, SDL_SCANCODE_LEFT },
    { ui::Action::eMenuRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT },
    { ui::Action::eMenuRight, SDL_SCANCODE_D },
    { ui::Action::eMenuRight, SDL_SCANCODE_RIGHT },
    { ui::Action::eMenuUp, SDL_CONTROLLER_BUTTON_DPAD_UP },
    { ui::Action::eMenuUp, SDL_SCANCODE_UP },
    { ui::Action::eMenuUp, SDL_SCANCODE_W },
};

constexpr std::tuple<GameAction, input::Actuator, input::Actuator> inputActions2[] = {
    { GameAction::eJetPitch, SDL_SCANCODE_W, SDL_SCANCODE_S },
    { GameAction::eJetYaw, SDL_SCANCODE_Q, SDL_SCANCODE_E },
    { GameAction::eJetRoll, SDL_SCANCODE_A, SDL_SCANCODE_D },
    { GameAction::eJetSpeed, SDL_SCANCODE_U, SDL_SCANCODE_O },
    { GameAction::eJetSpeed, SDL_CONTROLLER_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT },
};

Game::Game( int argc, char** argv )
: Engine{ Engine::CreateInfo{ .gameName = "starace", .versionMajor = 1, .versionMinor = 1, .argc = argc, .argv = argv } }
{
    ZoneScoped;
    g_uiProperty.m_textures = &m_textures;
    g_uiProperty.m_remapper = &m_remapper;
    loadSettings();
    m_io->setCallback( ".spv", []( Asset&& ) {} ); // HACK for loading dependant file in .mat
    m_io->setCallback( ".mat", this, &Game::loadMAT );
    m_io->setCallback( ".dds", this, &Game::loadDDS );
    m_io->setCallback( ".wav", this, &Game::loadWAV );
    m_io->setCallback( ".objc", this, &Game::loadOBJC );
    m_io->setCallback( ".lang", this, &Game::loadLANG );
    m_io->setCallback( ".map", this, &Game::loadMAP );
    m_io->setCallback( ".jet", this, &Game::loadJET );
    m_io->setCallback( ".wpn", this, &Game::loadWPN );
    m_io->setCallback( ".csg", this, &Game::loadCSG );
    m_io->setCallback( ".atlas", []( Asset&& a ) { g_uiProperty.loadATLAS( a.data ); } );
    m_io->setCallback( ".fnta", []( Asset&& a ) { g_uiProperty.loadFNTA( a.data ); } );
}

Game::~Game()
{
    ZoneScoped;
}

void Game::onExit()
{
}

void Game::onResize( uint32_t w, uint32_t h )
{
    ZoneScoped;
    if ( auto* screen = currentScreen() ) {
        screen->resize( { w, h } );
    }
}

void Game::onInit()
{
    ZoneScoped;

    m_io->mount( "init.pak" );
    createPipelines( ui::PIPELINES, g_uiProperty.setupPipeline() );

    m_screens.emplace_back( m_io->viewWait( "ui/loading.ui" ) );
    changeScreen( "loading"_hash );

    m_io->mount( "data.pak" );
    createPipelines( g_pipelineCreateInfo, []( auto p ) { g_pipelines[ static_cast<Pipeline>( p.first ) ] = p.second; } );
    g_pipelines[ Pipeline::eMesh ] = m_materials[ "mesh"_hash ];
    g_pipelines[ Pipeline::eProjectile ] = m_materials[ "projectile"_hash ];
    g_pipelines[ Pipeline::eThruster2 ] = m_materials[ "thruster2"_hash ];
    g_pipelines[ Pipeline::eParticleBlob ] = m_materials[ "particles"_hash ];
    g_pipelines[ Pipeline::eAfterglow ] = m_materials[ "afterglow"_hash ];

    auto addAction = [r=&m_remapper]( const auto& p )
    {
        auto [eid, act] = p;
        r->add( static_cast<input::Action::Enum>( eid ), act );
    };
    std::ranges::for_each( UI_INPUT, addAction );
    std::ranges::for_each( inputActions, addAction );
    std::ranges::for_each( inputActions2, [r=&m_remapper]( const auto& p ){
        auto [eid, min, max] = p;
        r->add( static_cast<input::Action::Enum>( eid ), min, max );
    } );

    m_click = m_sounds[ "sounds/click.wav" ];
    m_plasma = m_textures[ "textures/plasma.dds" ];
    m_enemyModel = Model{ m_meshes[ "models/a2.objc" ], m_textures[ "textures/a2.dds" ] };
    m_menuScene.setModel( &m_jetsContainer[ 0 ].model );

    applyGameSettings();
    setupUI();
    onResize( viewportWidth(), viewportHeight() );
    changeScreen( "mainMenu"_hash );
}

void Game::setupUI()
{
    m_optionsCustomize.m_jet.m_size = [this](){ return static_cast<ui::DataModel::size_type>( m_jetsContainer.size() ); };
    m_optionsCustomize.m_jet.m_at = [this]( auto i )
    {
        assert( i < m_jetsContainer.size() );
        return m_jetsContainer[ i ].name;
    };
    m_optionsCustomize.m_jet.m_select = [this]( auto i )
    {
        assert( i < m_jetsContainer.size() );
        m_currentJet = i;
        m_menuScene.setModel( &m_jetsContainer[ i ].model );
    };
    m_optionsCustomize.m_jet.m_current = [this]() { return m_currentJet; };
    m_optionsCustomize.m_jet.m_revision = [this]() { return m_currentJet; };

    auto weapNames = [this]( auto i ) -> std::pmr::u32string
    {
        assert( i < m_weapons.size() );
        auto key = m_weapons[ i ].displayName;
        return std::pmr::u32string{ g_uiProperty.localize( key ) };
    };
    auto weaponCount = [this](){ return static_cast<ui::DataModel::size_type>( m_weapons.size() ); };
    m_optionsCustomize.m_weaponPrimary.m_size = weaponCount;
    m_optionsCustomize.m_weaponPrimary.m_at = weapNames;
    m_optionsCustomize.m_weaponPrimary.m_select = [this]( auto i ){ m_weapon1 = i; };
    m_optionsCustomize.m_weaponPrimary.m_current = [this](){ return m_weapon1; };
    m_optionsCustomize.m_weaponPrimary.m_revision = [this](){ return m_weapon1; };
    m_optionsCustomize.m_weaponSecondary.m_size = weaponCount;
    m_optionsCustomize.m_weaponSecondary.m_at = weapNames;
    m_optionsCustomize.m_weaponSecondary.m_select = [this]( auto i ){ m_weapon2 = i; };
    m_optionsCustomize.m_weaponSecondary.m_current = [this](){ return m_weapon2; };
    m_optionsCustomize.m_weaponSecondary.m_revision = [this](){ return m_weapon2; };

    m_dataMissionSelect.m_current = [this](){ return m_currentMission; };
    m_dataMissionSelect.m_revision = [this](){ return m_currentMission; };
    m_dataMissionSelect.m_size = [this](){ return static_cast<ui::DataModel::size_type>( m_mapsContainer.size() ); };
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
    m_dataMissionSelect.m_texture = [this]( auto i ) -> ui::Sprite
    {
        assert( i < m_mapsContainer.size() );
        return ui::Sprite{ .xyuv{ 0.0f, 0.0f, 1.0f, 1.0f }, .texture = m_mapsContainer[ i ].preview };
    };

    m_optionsGFX.m_resolutionUI = ui::Option<DisplayMode>{ 0, displayModes(), &OptionsGFX::toString<DisplayMode> };

    {
        std::pmr::vector<AntiAlias> aa{ AntiAlias::eOff, AntiAlias::eFXAA };
        if ( m_renderer->featureAvailable( Renderer::Feature::eVRSAA ) ) {
            aa.emplace_back( AntiAlias::eVRSAA );
        }
        m_optionsGFX.m_antialiasUI = ui::Option<AntiAlias>{ 0, std::move( aa ), &OptionsGFX::toString<AntiAlias> };
    }
    {
        std::pmr::vector<VSync> v{ VSync::eOff, VSync::eOn };
        if ( m_renderer->featureAvailable( Renderer::Feature::eVSyncMailbox ) ) {
            v.emplace_back( VSync::eMailbox );
        }
        m_optionsGFX.m_vsyncUI = ui::Option<VSync>{ 1, std::move( v ), &OptionsGFX::toString<VSync> };
    };

    m_optionsAudio.m_driverNameUI.setData( 0, m_audio->listDrivers() );
    m_optionsAudio.m_deviceNameUI.setData( 0, m_audio->listDevices() );

    m_optionsGFX.settings2ui( m_gameSettings );
    m_optionsAudio.settings2ui( m_gameSettings );
    m_optionsGame.settings2ui( m_gameSettings );

    g_uiProperty.addDataModel( "$data:settings.display.fullscreen"_hash, &m_optionsGFX.m_fullscreenUI );
    g_uiProperty.addDataModel( "$data:settings.display.gamma"_hash, &m_optionsGFX.m_gammaUI );
    g_uiProperty.addDataModel( "$data:settings.display.resolution"_hash, &m_optionsGFX.m_resolutionUI );
    g_uiProperty.addDataModel( "$data:settings.display.vsync"_hash, &m_optionsGFX.m_vsyncUI );
    g_uiProperty.addDataModel( "$data:settings.display.antialias"_hash, &m_optionsGFX.m_antialiasUI );
    g_uiProperty.addDataModel( "$data:settings.display.fpsLimiter"_hash, &m_optionsGFX.m_fpsLimiterUI );
    g_uiProperty.addDataModel( "$data:settings.audio.driver"_hash, &m_optionsAudio.m_driverNameUI );
    g_uiProperty.addDataModel( "$data:settings.audio.device"_hash, &m_optionsAudio.m_deviceNameUI );
    g_uiProperty.addDataModel( "$data:settings.audio.master"_hash, &m_optionsAudio.m_masterUI );
    g_uiProperty.addDataModel( "$data:settings.audio.ui"_hash, &m_optionsAudio.m_uiUI );
    g_uiProperty.addDataModel( "$data:settings.audio.sfx"_hash, &m_optionsAudio.m_sfxUI );
    g_uiProperty.addDataModel( "$data:settings.game.language"_hash, &m_optionsGame.m_languageUI );
    g_uiProperty.addDataModel( "$data:weaponPrimary"_hash, &m_optionsCustomize.m_weaponPrimary );
    g_uiProperty.addDataModel( "$data:weaponSecondary"_hash, &m_optionsCustomize.m_weaponSecondary );
    g_uiProperty.addDataModel( "$data:jet"_hash, &m_optionsCustomize.m_jet );
    g_uiProperty.addDataModel( "$data:missionSelect"_hash, &m_dataMissionSelect );
    g_uiProperty.addDataModel( "$var:playerHP"_hash, &m_gameplayUIData.m_playerHP );
    g_uiProperty.addDataModel( "$var:playerReloadPrimary"_hash, &m_gameplayUIData.m_playerReloadPrimary );
    g_uiProperty.addDataModel( "$var:playerWeaponPrimaryCount"_hash, &m_gameplayUIData.m_playerWeaponPrimaryCount );
    g_uiProperty.addDataModel( "$var:playerWeaponSecondaryCount"_hash, &m_gameplayUIData.m_playerWeaponSecondaryCount );
    g_uiProperty.addDataModel( "$var:playerReloadSecondary"_hash, &m_gameplayUIData.m_playerReloadSecondary );
    g_uiProperty.addDataModel( "$var:playerWeaponPrimary"_hash, &m_gameplayUIData.m_playerWeaponIconPrimary );
    g_uiProperty.addDataModel( "$var:playerWeaponSecondary"_hash, &m_gameplayUIData.m_playerWeaponIconSecondary );
    g_uiProperty.addDataModel( "$var:jetSpeed"_hash, &m_gameplayUIData.m_jetSpeed );
    g_uiProperty.addDataModel( "$var:missionResult"_hash, &m_uiMissionResult );
    g_uiProperty.addDataModel( "$var:missionScore"_hash, &m_uiMissionScore );
    g_uiProperty.addCallback( "$function:goto_missionBriefing"_hash,  [this](){ changeScreen( "missionBriefing"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_newgame"_hash, [this](){ changeScreen( "missionSelect"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_customize"_hash, [this](){ changeScreen( "customize"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_titlemenu"_hash, [this](){ changeScreen( "mainMenu"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_settings"_hash, [this](){ changeScreen( "settings"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_settings_display"_hash, [this](){ changeScreen( "settings.display"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_settings_audio"_hash, [this](){ changeScreen( "settings.audio"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:goto_settings_game"_hash, [this](){ changeScreen( "settings.game"_hash, m_click ); } );
    g_uiProperty.addCallback( "$function:quit"_hash, [this](){ quit(); } );
    g_uiProperty.addCallback( "$function:resume"_hash, [this]{ changeScreen( "gameplay"_hash, m_click ); m_gameScene.setPause( false ); } );
    g_uiProperty.addCallback( "$function:applyDisplay"_hash, [this]()
    {
        if ( !m_optionsGFX.hasChanges( m_gameSettings ) ) return;
        m_optionsGFX.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyDisplay();
    });
    g_uiProperty.addCallback( "$function:exitDisplay"_hash, [this]()
    {
        if ( !m_optionsGFX.hasChanges( m_gameSettings ) ) return changeScreen( "settings"_hash, m_click );
        auto* screen = currentScreen();
        assert( screen );
        auto msg = screen->messageBox( "settings.display.unsaved"_hash );
        assert( msg );
        msg->addButton( "yes"_hash, ui::Action::eMenuConfirm, [this]()
        {
            m_optionsGFX.settings2ui( m_gameSettings );
            changeScreen( "settings"_hash, m_click );
        } );
        msg->addButton( "no"_hash, ui::Action::eMenuCancel, [screen]() { screen->addModalWidget( {} ); } );
        screen->addModalWidget( std::move( msg ) );
    });
    g_uiProperty.addCallback( "$function:applyAudio"_hash, [this]()
    {
        if ( !m_optionsAudio.hasChanges( m_gameSettings ) ) return;
        m_optionsAudio.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyAudio();
    } );
    g_uiProperty.addCallback( "$function:exitAudio"_hash, [this]()
    {
        if ( !m_optionsAudio.hasChanges( m_gameSettings ) ) return changeScreen( "settings"_hash, m_click );
        auto* screen = currentScreen();
        assert( screen );
        auto msg = screen->messageBox( "settings.audio.unsaved"_hash );
        assert( msg );
        msg->addButton( "yes"_hash, ui::Action::eMenuConfirm, [this]()
        {
            m_optionsAudio.settings2ui( m_gameSettings );
            changeScreen( "settings"_hash, m_click );
        } );
        msg->addButton( "no"_hash, ui::Action::eMenuCancel, [screen]() { screen->addModalWidget( {} ); } );
        screen->addModalWidget( std::move( msg ) );
    } );
    g_uiProperty.addCallback( "$function:applyGameSettings"_hash, [this]()
    {
        if ( !m_optionsGame.hasChanges( m_gameSettings ) ) return;
        m_optionsGame.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyGameSettings();
    });
    g_uiProperty.addCallback( "$function:exitGameSettings"_hash, [this]()
    {
        if ( !m_optionsGame.hasChanges( m_gameSettings ) ) return changeScreen( "settings"_hash, m_click );
        auto* screen = currentScreen();
        assert( screen );
        auto msg = screen->messageBox( "settings.game.unsaved"_hash );
        assert( msg );
        msg->addButton( "yes"_hash, ui::Action::eMenuConfirm, [this]()
        {
            m_optionsGame.settings2ui( m_gameSettings );
            changeScreen( "settings"_hash, m_click );
        } );
        msg->addButton( "no"_hash, ui::Action::eMenuCancel, [screen]() { screen->addModalWidget( {} ); } );
        screen->addModalWidget( std::move( msg ) );
    });

    m_gameplayUIData.m_playerWeaponIconPrimary = g_uiProperty.sprite( "icon.laser"_hash );
    m_gameplayUIData.m_playerWeaponIconSecondary = g_uiProperty.sprite( "icon.laser"_hash );
    m_screens.emplace_back( m_io->viewWait( "ui/customize.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/gameplay.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/result.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/missionselect.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/pause.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/settings.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/settings_display.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/settings_audio.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/settings_game.ui" ) );
    m_screens.emplace_back( m_io->viewWait( "ui/mainmenu.ui" ) );

    m_menuScene = MenuScene{ MenuScene::CreateInfo{
        .background = g_uiProperty.sprite( "background"_hash ),
        .pipeline = m_materials[ "background"_hash ],
        .spaceDustPipeline = m_materials[ "space_dust"_hash ],
    } };
    m_menuScene.setModel( &m_jetsContainer[ 0 ].model );
}

ui::Screen* Game::currentScreen()
{
    return m_currentScreen;
}

void Game::onRender( Renderer* renderer )
{
    ui::Screen* screen = currentScreen();
    if ( !screen ) [[unlikely]] return;

    const auto [ width, height, aspect ] = viewport();
    switch ( screen->scene() ) {
    case "gameplay"_hash:
    case "pause"_hash:
        m_gameScene.render( renderer, math::vec2{ width, height } );
        break;
    case "menu"_hash:
        m_menuScene.render( renderer, math::vec2{ width, height } );
        break;
    case 0: break;
    default:
        assert( !"unhandled scene" );
        break;
    }

    screen->render( renderer, math::vec2( width, height ) );
    if ( screen->scene() == 0 ) [[unlikely]] return;

    switch ( m_gameSettings.antialias ) {
    case AntiAlias::eFXAA:
    case AntiAlias::eVRSAA: {
        const PushConstant<Pipeline::eAntiAliasFXAA> aa{};
        const DispatchInfo daa{
            .m_pipeline = g_pipelines[ Pipeline::eAntiAliasFXAA ],
            .m_uniform = aa,
        };
        renderer->dispatch( daa );
    } break;
    default:
        break;
    }

    const PushConstant<Pipeline::eGammaCorrection> gp{ .m_power = m_gameSettings.gamma, };
    const DispatchInfo dispatchInfo{
        .m_pipeline = g_pipelines[ Pipeline::eGammaCorrection ],
        .m_uniform = gp,
    };
    renderer->dispatch( dispatchInfo );
}

void Game::onUpdate( float deltaTime )
{
    ZoneScoped;
    ui::Screen* screen = currentScreen();
    if ( !screen ) [[unlikely]] {
        return;
    }

    UpdateContext uctx{ .deltaTime = deltaTime, };

    switch ( screen->scene() ) {
    case "gameplay"_hash:
    case "pause"_hash:
        updateGame( uctx );
        break;
    case "menu"_hash:
        m_menuScene.update( uctx );
        break;
    case 0:
        break;
    default:
        assert( !"unhandled scene" );
        break;
    }
    ui::UpdateContext uictx{ .deltaTime = deltaTime, };
    screen->update( uictx );
}

void Game::updateGame( UpdateContext& updateContext )
{
    ZoneScoped;

    if ( m_gameScene.isPause() ) return;
    if ( m_gameScene.player().status() == Player::Status::eDead ) {
        changeScreen( "lose"_hash );
        m_gameScene.setPause( true );
        return;
    }
    if ( m_gameScene.enemies().empty() ) {
        changeScreen( "win"_hash );
        m_gameScene.setPause( true );
        return;
    }

    m_gameScene.update( updateContext );
    m_gameplayUIData.m_playerHP = static_cast<float>( m_gameScene.player().health() ) / 100.0f;
    const math::vec2 reloadState = m_gameScene.player().reloadState();
    m_gameplayUIData.m_playerReloadPrimary = reloadState.x;
    m_gameplayUIData.m_playerReloadSecondary = reloadState.y;
    m_gameplayUIData.m_jetSpeed = m_gameScene.player().speed() / 1600_kmph;
    auto [ primary, secondary ] = m_gameScene.player().weaponClip();
    m_gameplayUIData.m_playerWeaponPrimaryCount = primary;
    m_gameplayUIData.m_playerWeaponSecondaryCount = secondary;

}

void Game::createMapData( const MapCreateInfo& mapInfo, const ModelProto& modelData )
{
    ZoneScoped;
    const auto& w1 = m_weapons[ m_weapon1 ];
    const auto& w2 = m_weapons[ m_weapon2 ];
    m_gameScene = GameScene{ GameScene::CreateInfo{
        .audio = m_audio,
        .skybox = mapInfo.texture,
        .plasma = m_plasma,
        .enemyModel = &m_enemyModel,
        .enemyWeapon = m_enemyWeapon,
        .enemyCallsigns = m_callsigns,
        .player = Player::CreateInfo{
            .model = modelData.model,
            .weapons{ w1, w2, w1 },
        },
        .spaceDustPipeline = m_materials[ "space_dust"_hash ],
    } };

    m_gameplayUIData.m_playerWeaponIconPrimary = g_uiProperty.sprite( w1.displayIcon );
    m_gameplayUIData.m_playerWeaponIconSecondary = g_uiProperty.sprite( w2.displayIcon );
}

void Game::changeScreen( Hash::value_type screenId, Audio::Slot sound )
{
    ZoneScoped;
    auto setScreen = [this]( auto hash )
    {
        auto it = std::ranges::find_if( m_screens, [hash]( const auto& sc ) { return sc.name() == hash; } );
        assert( it != m_screens.end() );
        m_currentScreen = &*it;
        auto [ w, h, a ] = viewport();
        m_currentScreen->show( { w, h } );
    };

    if ( sound ) {
        m_audio->play( sound, Audio::Channel::eUI );
    }

    SDL_ShowCursor( screenId != "gameplay"_hash );
    switch ( screenId ) {
    case "lose"_hash:
        m_uiMissionResult = std::pmr::u32string{ g_uiProperty.localize( "missionLost"_hash ) };
        m_uiMissionScore = std::pmr::u32string { g_uiProperty.localize( "yourScore"_hash ) } + intToUTF32( m_gameScene.score() );
        setScreen( "result"_hash );
        break;
    case "win"_hash:
        m_uiMissionResult = std::pmr::u32string{ g_uiProperty.localize( "missionWin"_hash ) };
        m_uiMissionScore = std::pmr::u32string{ g_uiProperty.localize( "yourScore"_hash ) } + intToUTF32( m_gameScene.score() );
        setScreen( "result"_hash );
        break;
    case "missionBriefing"_hash:
        createMapData( m_mapsContainer[ m_currentMission ], m_jetsContainer[ m_currentJet ] );
        setScreen( "pause"_hash );
        break;
    default:
        setScreen( screenId );
        break;
    }
}

void Game::loadOBJC( Asset&& asset )
{
    auto&& [ it, inserted ] = m_meshes.insert( std::make_pair( asset.path, Mesh{} ) );
    if ( !inserted ) return;
    it->second = Mesh{ asset.data, m_renderer };
}

void Game::loadMAP( Asset&& asset )
{
    using std::string_view_literals::operator""sv;
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( asset.data );
    MapCreateInfo& ci = m_mapsContainer.emplace_back();
    ci.texture[ MapCreateInfo::eTop ] = m_textures[ entry[ "top"sv ].toString() ];
    ci.texture[ MapCreateInfo::eBottom ] = m_textures[ entry[ "bottom"sv ].toString() ];
    ci.texture[ MapCreateInfo::eLeft ] = m_textures[ entry[ "left"sv ].toString() ];
    ci.texture[ MapCreateInfo::eRight ] = m_textures[ entry[ "right"sv ].toString() ];
    ci.texture[ MapCreateInfo::eFront ] = m_textures[ entry[ "front"sv ].toString() ];
    ci.texture[ MapCreateInfo::eBack ] = m_textures[ entry[ "back"sv ].toString() ];
    ci.name = entry[ "name"sv ].toString32();
    ci.preview = m_textures[ entry[ "preview"sv ].toString() ];
    ci.enemies = entry[ "enemies"sv ].toInt<uint32_t>();
}

void Game::loadJET( Asset&& asset )
{
    using std::string_view_literals::operator""sv;
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( asset.data );
    auto& jet = m_jetsContainer.emplace_back();
    auto&& texture = m_textures[ entry[ "texture"sv ].toString() ];
    auto&& mesh = m_meshes[ entry[ "model"sv ].toString() ];
    jet.model = Model{ mesh, texture };
    jet.name = entry[ "name"sv ].toString32();
}

void Game::loadWPN( Asset&& asset )
{
    ZoneScoped;
    auto makeType = []( std::string_view sv )
    {
        Hash hash{};
        switch ( hash( sv ) ) {
        default:
            assert( !"unknown type" );
            [[fallthrough]];
        case "blaster"_hash: return Bullet::Type::eBlaster;
        case "torpedo"_hash: return Bullet::Type::eTorpedo;
        case "laser"_hash: return Bullet::Type::eLaser;
        }
    };

    auto entry = cfg::Entry::fromData( asset.data );
    Hash hash{};
    WeaponCreateInfo weap{};
    bool isHidden = false;
    for ( const auto& property : entry ) {
        switch ( hash( property.name() ) ) {
        case "damage"_hash: weap.damage = property.toInt<uint8_t>(); continue;
        case "delay"_hash: weap.delay = property.toFloat(); continue;
        case "reload"_hash: weap.reload = property.toFloat(); continue;
        case "capacity"_hash: weap.capacity = property.toInt<uint16_t>(); continue;
        case "hidden"_hash: isHidden = property.toInt<bool>(); continue;
        case "speed"_hash: weap.speed = property.toFloat() * (float)meter; continue;
        case "distance"_hash: weap.distance = property.toFloat() * (float)meter; continue;
        case "loc"_hash: weap.displayName = hash( property.toString() ); continue;
        case "score"_hash: weap.score_per_hit = property.toInt<uint16_t>(); continue;
        case "type"_hash: weap.type = makeType( property.toString() ); continue;
        case "icon"_hash: weap.displayIcon = hash( property.toString() ); continue;
        case "mesh"_hash: weap.mesh = m_meshes[ property.toString() ][ "projectile" ]; continue;
        case "texture"_hash: weap.texture = m_textures[ property.toString() ]; continue;
        case "sound"_hash: weap.sound = m_sounds[ property.toString() ]; continue;
        default:
            assert( !"unknown weapon property" );
            continue;
        }
    }
    if ( isHidden ) m_enemyWeapon = weap;
    else m_weapons.emplace_back( weap );
}

void Game::loadLANG( Asset&& asset )
{
    ZoneScoped;
    auto& ll = g_uiProperty.m_lockit.emplace_back( asset.data );
    m_optionsGame.m_languageUI.addOption( OptionsGame::LanguageInfo{
        .id = ll.id(),
        .display = std::pmr::u32string{ ll.find( "lockit"_hash ) },
    } );
}

void Game::loadCSG( Asset&& asset )
{
    csg::Header header;
    if ( !asset.read( header ) ) {
        assert( !"callsign file corrupted, not enough data to read header" );
        return;
    }
    assert( header.magic == header.MAGIC );
    assert( header.version == header.VERSION );
    std::pmr::vector<csg::Callsign> callsigns{};
    auto size = m_callsigns.size();
    m_callsigns.resize( size + header.count );
    std::memcpy( m_callsigns.data() + size, asset.data.data(), header.count * sizeof( csg::Callsign ) );
}

void Game::applyDisplay()
{
    DisplayMode displayMode = m_gameSettings.resolution;
    displayMode.fullscreen = m_gameSettings.fullscreen;
    m_renderer->setVSync( m_gameSettings.vsync );
    m_renderer->setFeatureEnabled( Renderer::Feature::eVRSAA, m_gameSettings.antialias == AntiAlias::eVRSAA );
    setDisplayMode( displayMode );
    setTargetFPS( 200, m_gameSettings.fpsLimiter ? FpsLimiter::eSpinLock : FpsLimiter::eOff );
}

void Game::applyAudio()
{
    m_audio->setVolume( Audio::Channel::eMaster, m_gameSettings.audioMaster );
    m_audio->setVolume( Audio::Channel::eSFX, m_gameSettings.audioSFX );
    m_audio->setVolume( Audio::Channel::eUI, m_gameSettings.audioUI );
    m_audio->selectDriver( m_gameSettings.audioDriverName );
    m_audio->selectDevice( m_gameSettings.audioDeviceName );
}

void Game::applyGameSettings()
{
    if ( m_gameSettings.gameLang[ 0 ] == 0 ) {
        const auto& ll = g_uiProperty.m_lockit.front();
        std::ranges::copy( ll.id(), std::begin( m_gameSettings.gameLang ) );
    }
    else {
        auto it = std::ranges::find_if( g_uiProperty.m_lockit, [id=m_gameSettings.gameLang]( const auto& l )
        {
            return l.id() == id;
        } );
        assert( it != g_uiProperty.m_lockit.end() );
        g_uiProperty.m_currentLang = static_cast<uint32_t>( std::distance( g_uiProperty.m_lockit.begin(), it ) );
        m_optionsGame.m_languageUI.select( (uint16_t)g_uiProperty.m_currentLang );
        std::ranges::for_each( m_screens, []( auto& s ) { s.lockitChanged(); } );
    }
}

void Game::loadSettings()
{
    [this]()
    {
        std::pmr::vector<uint8_t> config{};
        GameSettings saveConfig{};
        if ( m_saveSystem->load( 0, config ) != SaveSystem::eSuccess ) return;
        if ( config.size() != sizeof( saveConfig ) ) return;
        std::memcpy( &saveConfig, config.data(), sizeof( saveConfig ) );
        if ( saveConfig.magic != saveConfig.MAGIC ) return;
        if ( saveConfig.version != saveConfig.VERSION ) return;
        m_gameSettings = saveConfig;
    }();

    // validate
    if ( auto&& ds = displayModes(); !validate( m_gameSettings.resolution, ds ) ) {
        assert( !ds.empty() );
        if ( !ds.empty() ) {
            m_gameSettings.resolution = ds.front();
        }
    }
    if ( auto&& devs = m_audio->listDevices(); !validate( m_gameSettings.audioDeviceName, devs ) ) {
        assert( !devs.empty() );
        if ( !devs.empty() ) {
            copySecure( devs.front(), m_gameSettings.audioDeviceName );
        }
    }
    if ( auto&& devs = m_audio->listDrivers(); !validate( m_gameSettings.audioDriverName, devs ) ) {
        assert( !devs.empty() );
        if ( !devs.empty() ) {
            copySecure( devs.front(), m_gameSettings.audioDriverName );
        }
    }
    applyDisplay();
    applyAudio();
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

void Game::onAction( input::Action a )
{
    using namespace input;
    ZoneScoped;

    ui::Screen* screen = currentScreen();
    assert( screen );
    if ( a.testEnumRange<(Action::Enum)ui::Action::base, (Action::Enum)ui::Action::end>() ) {
        screen->onAction( ui::Action{ .a = a.toA<ui::Action::Enum>(), .value = a.value } );
        return;
    }

    if ( screen->scene() != "gameplay"_hash ) return;
    const GameAction action = a.toA<GameAction>();
    switch ( action ) {
    case GameAction::eGamePause:
        if ( !a.digital() || !a.value ) break;
        m_gameScene.setPause( true );
        changeScreen( "pause"_hash );
        break;
    default:
        m_gameScene.onAction( a );
        break;
    }
}

void Game::onMouseEvent( const MouseEvent& mouseEvent )
{
    using namespace input;
    const bool inputSourceChanges = g_uiProperty.setInputSource( Actuator::Source::eKBM );
    ZoneScoped;
    switch ( mouseEvent.type ) {
    case MouseEvent::eClickSecondary:
        onAction( Action{ .userEnum = (Action::Enum)ui::Action::eMenuCancel, .value = mouseEvent.value } );
        return;
    case MouseEvent::eClickMiddle:
        return;
    case MouseEvent::eClick:
        if ( mouseEvent.value == 0 ) return;
    case MouseEvent::eMove: break;
    }
    ui::Screen* screen = currentScreen();
    if ( !screen ) return;
    if ( inputSourceChanges ) {
        screen->refreshInput();
    }
    screen->onMouseEvent( mouseEvent );
}

void Game::onActuator( input::Actuator a )
{
    using namespace input;
    bool inputChanged = g_uiProperty.setInputSource( a.source );
    if ( ui::Screen* screen = currentScreen(); inputChanged && screen ) {
        screen->refreshInput();
    }

    const std::pmr::vector<Action> actions = m_remapper.updateAndResolve( a );
    for ( const auto& it : actions ) {
        onAction( it );
    }
}
