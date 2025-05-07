#include "game.hpp"

#include "colors.hpp"
#include "game_action.hpp"
#include "game_callbacks.hpp"
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
#include <cmath>
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
: Engine{ argc, argv }
{
    ZoneScoped;
    loadSettings();
    m_io->setCallback( ".dds", [this]( const auto& asset ){ loadDDS( asset ); } );
    m_io->setCallback( ".objc", [this]( const auto& asset ){ loadOBJC( asset ); } );
    m_io->setCallback( ".wav", [this]( const auto& asset ){ loadWAV( asset ); } );
    m_io->setCallback( ".lang", [this]( const auto& asset ){ loadLANG( asset ); } );
    m_io->setCallback( ".map", [this]( const auto& asset ){ loadMAP( asset ); } );
    m_io->setCallback( ".jet", [this]( const auto& asset ){ loadJET( asset ); } );
    m_io->setCallback( ".wpn", [this]( const auto& asset ){ loadWPN( asset ); } );
    m_io->setCallback( ".csg", [this]( const auto& asset ){ loadCSG( asset ); } );
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
    auto setupPipeline = []( auto* renderer, auto* io, auto ci ) -> PipelineSlot
    {
        if ( ci.m_vertexShader ) ci.m_vertexShaderData = io->viewWait( ci.m_vertexShader );
        if ( ci.m_fragmentShader ) ci.m_fragmentShaderData = io->viewWait( ci.m_fragmentShader );
        if ( ci.m_computeShader ) ci.m_computeShaderData = io->viewWait( ci.m_computeShader );
        return renderer->createPipeline( ci );
    };

    m_io->mount( "init.pak" );
    g_uiProperty.m_pipelineSpriteSequence = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE );
    g_uiProperty.m_atlas = &m_uiAtlas;
    m_uiAtlas = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "misc/ui_atlas.fnta" ),
        .texture = m_textures[ "textures/atlas_ui.dds" ],
    };
    m_screens.emplace_back( m_io->viewWait( "ui/loading.ui" ) );
    changeScreen( "loading"_hash );

    m_io->mount( "data.pak" );

    for ( const auto& p : g_pipelineCreateInfo ) {
        g_pipelines[ static_cast<Pipeline>( p.m_userHint ) ] = setupPipeline( m_renderer, m_io, p );
    }

    g_uiProperty.m_pipelineSpriteSequenceColors = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE_COLORS );
    g_uiProperty.m_pipelineGlow = setupPipeline( m_renderer, m_io, ui::GLOW );
    g_uiProperty.m_pipelineBlurDesaturate = setupPipeline( m_renderer, m_io, ui::BLUR_DESATURATE );

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

    m_dustUi.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_dustUi.setCenter( {} );
    m_dustUi.setLineWidth( 2.0f );

    m_click = m_sounds[ "sounds/click.wav" ];
    m_plasma = m_textures[ "textures/plasma.dds" ];
    m_enemyModel = Model{ m_meshes[ "models/a2.objc" ], m_textures[ "textures/a2.dds" ] };

    applyGameSettings();
    setupUI();
    onResize( viewportWidth(), viewportHeight() );
    changeScreen( "mainMenu"_hash );
}

void Game::setupUI()
{

    m_inputPS4 = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "misc/ps4_atlas.fnta" ),
        .texture = m_textures[ "textures/ps4_atlas.dds" ],
    };
    m_inputXbox = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "misc/xbox_atlas.fnta" ),
        .upstream = &m_inputPS4,
        .texture = m_textures[ "textures/xbox_atlas.dds" ],
    };
    m_fontSmall = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_24.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_remapper,
        .texture = m_textures[ "fonts/dejavu_24.dds" ],
        .scale = 0.5f,
    };
    m_fontMedium = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_36.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_remapper,
        .texture = m_textures[ "fonts/dejavu_36.dds" ],
        .scale = 0.5f,
    };
    m_fontLarge = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_64.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_remapper,
        .texture = m_textures[ "fonts/dejavu_64.dds" ],
        .scale = 0.5f,
    };

    g_uiProperty.m_fontSmall = &m_fontSmall;
    g_uiProperty.m_fontMedium = &m_fontMedium;
    g_uiProperty.m_fontLarge = &m_fontLarge;

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
    m_dataMissionSelect.m_texture = [this]( auto i ) -> Texture
    {
        assert( i < m_mapsContainer.size() );
        return m_mapsContainer[ i ].preview;
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

    m_gameUiDataModels.insert( "$data:settings.display.fullscreen"_hash, &m_optionsGFX.m_fullscreenUI );
    m_gameUiDataModels.insert( "$data:settings.display.gamma"_hash, &m_optionsGFX.m_gammaUI );
    m_gameUiDataModels.insert( "$data:settings.display.resolution"_hash, &m_optionsGFX.m_resolutionUI );
    m_gameUiDataModels.insert( "$data:settings.display.vsync"_hash, &m_optionsGFX.m_vsyncUI );
    m_gameUiDataModels.insert( "$data:settings.display.antialias"_hash, &m_optionsGFX.m_antialiasUI );
    m_gameUiDataModels.insert( "$data:settings.display.fpsLimiter"_hash, &m_optionsGFX.m_fpsLimiterUI );
    m_gameUiDataModels.insert( "$data:settings.audio.driver"_hash, &m_optionsAudio.m_driverNameUI );
    m_gameUiDataModels.insert( "$data:settings.audio.device"_hash, &m_optionsAudio.m_deviceNameUI );
    m_gameUiDataModels.insert( "$data:settings.audio.master"_hash, &m_optionsAudio.m_masterUI );
    m_gameUiDataModels.insert( "$data:settings.audio.ui"_hash, &m_optionsAudio.m_uiUI );
    m_gameUiDataModels.insert( "$data:settings.audio.sfx"_hash, &m_optionsAudio.m_sfxUI );
    m_gameUiDataModels.insert( "$data:settings.game.language"_hash, &m_optionsGame.m_languageUI );
    m_gameUiDataModels.insert( "$data:weaponPrimary"_hash, &m_optionsCustomize.m_weaponPrimary );
    m_gameUiDataModels.insert( "$data:weaponSecondary"_hash, &m_optionsCustomize.m_weaponSecondary );
    m_gameUiDataModels.insert( "$data:jet"_hash, &m_optionsCustomize.m_jet );
    m_gameUiDataModels.insert( "$data:missionSelect"_hash, &m_dataMissionSelect );
    m_gameUiDataModels.insert( "$var:playerHP"_hash, &m_gameplayUIData.m_playerHP );
    m_gameUiDataModels.insert( "$var:playerReloadPrimary"_hash, &m_gameplayUIData.m_playerReloadPrimary );
    m_gameUiDataModels.insert( "$var:playerWeaponPrimaryCount"_hash, &m_gameplayUIData.m_playerWeaponPrimaryCount );
    m_gameUiDataModels.insert( "$var:playerWeaponSecondaryCount"_hash, &m_gameplayUIData.m_playerWeaponSecondaryCount );
    m_gameUiDataModels.insert( "$var:playerReloadSecondary"_hash, &m_gameplayUIData.m_playerReloadSecondary );
    m_gameUiDataModels.insert( "$var:playerWeaponPrimary"_hash, &m_gameplayUIData.m_playerWeaponIconPrimary );
    m_gameUiDataModels.insert( "$var:playerWeaponSecondary"_hash, &m_gameplayUIData.m_playerWeaponIconSecondary );
    m_gameUiDataModels.insert( "$var:jetSpeed"_hash, &m_gameplayUIData.m_jetSpeed );
    m_gameUiDataModels.insert( "$var:missionResult"_hash, &m_uiMissionResult );
    m_gameUiDataModels.insert( "$var:missionScore"_hash, &m_uiMissionScore );
    m_gameCallbacks.insert( "$function:goto_missionBriefing"_hash,  [this](){ changeScreen( "missionBriefing"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_newgame"_hash, [this](){ changeScreen( "missionSelect"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_customize"_hash, [this](){ changeScreen( "customize"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_titlemenu"_hash, [this](){ changeScreen( "mainMenu"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings"_hash, [this](){ changeScreen( "settings"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings_display"_hash, [this](){ changeScreen( "settings.display"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings_audio"_hash, [this](){ changeScreen( "settings.audio"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings_game"_hash, [this](){ changeScreen( "settings.game"_hash, m_click ); } );
    m_gameCallbacks.insert( "$function:quit"_hash, [this](){ quit(); } );
    m_gameCallbacks.insert( "$function:resume"_hash, [this]{ changeScreen( "gameplay"_hash, m_click ); m_gameScene.setPause( false ); } );
    m_gameCallbacks.insert( "$function:applyDisplay"_hash, [this]()
    {
        if ( !m_optionsGFX.hasChanges( m_gameSettings ) ) return;
        m_optionsGFX.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyDisplay();
    });
    m_gameCallbacks.insert( "$function:exitDisplay"_hash, [this]()
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
    m_gameCallbacks.insert( "$function:applyAudio"_hash, [this]()
    {
        if ( !m_optionsAudio.hasChanges( m_gameSettings ) ) return;
        m_optionsAudio.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyAudio();
    } );
    m_gameCallbacks.insert( "$function:exitAudio"_hash, [this]()
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
    m_gameCallbacks.insert( "$function:applyGameSettings"_hash, [this]()
    {
        if ( !m_optionsGame.hasChanges( m_gameSettings ) ) return;
        m_optionsGame.ui2settings( m_gameSettings );
        m_saveSystem->save( 0, m_gameSettings );
        applyGameSettings();
    });
    m_gameCallbacks.insert( "$function:exitGameSettings"_hash, [this]()
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

    g_uiProperty.m_gameCallbacks = m_gameCallbacks.makeView();
    g_uiProperty.m_dataModels = m_gameUiDataModels.makeView();

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

    Targeting::CreateInfo tci{
        .callsigns = m_callsigns,
    };
    m_targeting = Targeting{ tci };
}

ui::Screen* Game::currentScreen()
{
    return m_currentScreen;
}

void Game::onRender( Renderer* renderer )
{
    ui::Screen* screen = currentScreen();
    assert( screen );

    const auto [ width, height, aspect ] = viewport();
    const auto [ view, projection ] = m_gameScene.getCameraMatrix( aspect );
    RenderContext rctx{
        .renderer = renderer,
        .projection = math::ortho( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ), -100.0f, 100.0f ),
        .camera3d = projection * view,
        .viewport = { width, height },
    };
    const ui::RenderContext r{
        .renderer = rctx.renderer,
        .model = rctx.model,
        .view = rctx.view,
        .projection = rctx.projection,
        .colorMain = color::dodgerBlue,
        .colorFocus = color::lightSkyBlue,
    };

    switch ( screen->scene() ) {
    case "gameplay"_hash:
    case "pause"_hash:
        renderGameScreen( rctx );
        break;
    case "menu"_hash:
        renderMenuScreen( rctx, r );
        break;
    case 0: break;
    default:
        assert( !"unhandled scene" );
        break;
    }

    screen->render( r );
    if ( screen->scene() == 0 ) [[unlikely]] return;

    const PushConstant<Pipeline::eGammaCorrection> pushConstant{
        .m_power = m_gameSettings.gamma,
    };

    const DispatchInfo dispatchInfo{
        .m_pipeline = g_pipelines[ Pipeline::eGammaCorrection ],
    };
    m_renderer->dispatch( dispatchInfo, &pushConstant );
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
        m_dustUi.update( uctx );
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

    auto signals = m_gameScene.signals();
    updateContext.signals = signals;
    m_gameScene.update( updateContext );

    m_targeting.setSignals( std::move( signals ) );
    m_targeting.setTarget( m_gameScene.player().target(), m_gameScene.player().targetingState() );
    m_targeting.update( updateContext );
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
        .enemyCallsignCount = m_callsigns.size(),
        .player = Player::CreateInfo{
            .model = modelData.model,
            .weapons{ w1, w2, w1 },
        },
    } };

    m_gameplayUIData.m_playerWeaponIconPrimary = w1.displayIcon;
    m_gameplayUIData.m_playerWeaponIconSecondary = w2.displayIcon;
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

void Game::loadDDS( const Asset& asset )
{
    auto&& [ it, inserted ] = m_textures.insert( std::make_pair( asset.path, Texture{} ) );
    if ( !inserted ) return;
    it->second = parseTexture( asset.data );
}

void Game::loadOBJC( const Asset& asset )
{
    auto&& [ it, inserted ] = m_meshes.insert( std::make_pair( asset.path, Mesh{} ) );
    if ( !inserted ) return;
    it->second = Mesh{ asset.data, m_renderer };
}

void Game::loadMAP( const Asset& asset )
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

void Game::loadJET( const Asset& asset )
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

void Game::loadWPN( const Asset& asset )
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
        switch ( hash( *property ) ) {
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

void Game::loadLANG( const Asset& asset )
{
    ZoneScoped;
    auto& ll = g_uiProperty.m_lockit.emplace_back( asset.data );
    m_optionsGame.m_languageUI.addOption( OptionsGame::LanguageInfo{
        .id = ll.id(),
        .display = std::pmr::u32string{ ll.find( "lockit"_hash ) },
    } );
}

void Game::loadWAV( const Asset& asset )
{
    ZoneScoped;
    auto [ it, inserted ] = m_sounds.insert( std::make_pair( asset.path, Audio::Slot{} ) );
    if ( !inserted ) return;
    it->second = m_audio->load( asset.data );
}

void Game::loadCSG( const Asset& asset )
{
    csg::Header header;
    assert( asset.data.size() >= sizeof( header ) );
    std::memcpy( &header, asset.data.data(), sizeof( header ) );
    assert( header.magic == header.MAGIC );
    assert( header.version == header.VERSION );
    auto data = asset.data.subspan( sizeof( header ) );
    std::pmr::vector<csg::Callsign> callsigns{};
    auto size = m_callsigns.size();
    m_callsigns.resize( size + header.count );
    std::memcpy( m_callsigns.data() + size, data.data(), header.count * sizeof( csg::Callsign ) );
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

void Game::renderGameScreen( RenderContext rctx )
{
    render3D( rctx );
    m_targeting.render( rctx );
}

void Game::render3D( RenderContext rctx )
{
    std::tie( rctx.view, rctx.projection ) = m_gameScene.getCameraMatrix( viewportAspect() );
    std::tie( rctx.cameraPosition, rctx.cameraUp, std::ignore ) = m_gameScene.getCamera();

    m_gameScene.render( rctx );

    switch ( m_gameSettings.antialias ) {
    case AntiAlias::eFXAA:
    case AntiAlias::eVRSAA: {
        const PushConstant<Pipeline::eAntiAliasFXAA> aa{};
        const DispatchInfo daa{ .m_pipeline = g_pipelines[ Pipeline::eAntiAliasFXAA ] };
        rctx.renderer->dispatch( daa, &aa );
    } break;
    default:
        break;
    }
}

void Game::renderBackground( ui::RenderContext rctx ) const
{
    ZoneScoped;
    [[maybe_unused]]
    const auto [ w, h, a ] = viewport();
    const math::vec2 uv = math::vec2{ w, h } / m_uiAtlas.extent();

    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eBackground ],
        .m_verticeCount = 4,
    };
    pushBuffer.m_fragmentTexture[ 1 ] = g_uiProperty.atlasTexture();

    PushConstant<Pipeline::eBackground> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = rctx.colorMain,
        .m_uvSlice = m_uiAtlas[ "background"_hash ] / m_uiAtlas.extent(),
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

    switch ( m_gameSettings.antialias ) {
    case AntiAlias::eFXAA:
    case AntiAlias::eVRSAA: {
        const PushConstant<Pipeline::eAntiAliasFXAA> aa{};
        const DispatchInfo daa{ .m_pipeline = g_pipelines[ Pipeline::eAntiAliasFXAA ] };
        rctx.renderer->dispatch( daa, &aa );
    } break;
    default:
        break;
    }
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
