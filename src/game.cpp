#include "game.hpp"

#include "colors.hpp"
#include "game_action.hpp"
#include "game_callbacks.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"
#include <ui/property.hpp>
#include <ui/pipeline.hpp>

#include <config/config.hpp>

#include <profiler.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <random>
#include <cmath>
#include <set>

constexpr std::tuple<GameAction, Actuator> inputActions[] = {
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

constexpr std::tuple<ui::Action::Enum, Actuator> UI_INPUT[] = {
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
    clearMapData();
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
    m_io->mount( "data.pak" );

    auto setupPipeline = []( auto* renderer, auto* io, auto ci ) -> PipelineSlot
    {
        if ( ci.m_vertexShader ) ci.m_vertexShaderData = io->viewWait( ci.m_vertexShader );
        if ( ci.m_fragmentShader ) ci.m_fragmentShaderData = io->viewWait( ci.m_fragmentShader );
        if ( ci.m_computeShader ) ci.m_computeShaderData = io->viewWait( ci.m_computeShader );
        return renderer->createPipeline( ci );
    };

    for ( const auto& p : g_pipelineCreateInfo ) {
        g_pipelines[ static_cast<Pipeline>( p.m_userHint ) ] = setupPipeline( m_renderer, m_io, p );
    }

    g_uiProperty.m_pipelineSpriteSequence = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE );
    g_uiProperty.m_pipelineSpriteSequenceColors = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE_COLORS );
    g_uiProperty.m_pipelineGlow = setupPipeline( m_renderer, m_io, ui::GLOW );

    m_enemies.reserve( 100 );
    m_bullets.reserve( 2000 );
    for ( auto [ eid, act ] : inputActions ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), act );
    }
    for ( auto [ eid, act ] : UI_INPUT ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), act );
    }
    for ( auto [ eid, min, max ] : inputActions2 ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), min, max );
    }

    m_dustUi.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_dustUi.setCenter( {} );
    m_dustUi.setLineWidth( 2.0f );

    m_click = m_sounds[ "sounds/click.wav" ];
    m_plasma = m_textures[ "textures/plasma.dds" ];
    m_enemyModel = Model{ m_meshes[ "models/a2.objc" ], m_textures[ "textures/a2.dds" ] };

    setupUI();
    onResize( viewportWidth(), viewportHeight() );
    changeScreen( Scene::eMainMenu );
}

void Game::setupUI()
{
    m_uiAtlas = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "misc/ui_atlas.fnta" ),
        .texture = m_textures[ "textures/atlas_ui.dds" ],
    };
    m_inputXbox = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "misc/xbox_atlas.fnta" ),
        .texture = m_textures[ "textures/xbox_atlas.dds" ],
    };
    m_fontSmall = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_24.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_uiRemapper,
        .texture = m_textures[ "fonts/dejavu_24.dds" ],
        .scale = 0.5f,
    };
    m_fontMedium = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_36.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_uiRemapper,
        .texture = m_textures[ "fonts/dejavu_36.dds" ],
        .scale = 0.5f,
    };
    m_fontLarge = ui::Font::CreateInfo{
        .fontAtlas = m_io->viewWait( "fonts/dejavu_64.fnta" ),
        .upstream = &m_inputXbox,
        .remapper = &m_uiRemapper,
        .texture = m_textures[ "fonts/dejavu_64.dds" ],
        .scale = 0.5f,
    };

    g_uiProperty.m_atlas = &m_uiAtlas;
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
        return g_uiProperty.localize( key );
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

    m_optionsGFX.m_resolution = ui::Option<DisplayMode>{ 0, displayModes(), &OptionsGFX::toString<DisplayMode> };

    {
        using enum OptionsGFX::AntiAlias;
        std::pmr::vector<OptionsGFX::AntiAlias> aa{ eOff, eFXAA };
        if ( m_renderer->featureAvailable( Renderer::Feature::eVRSAA ) ) {
            aa.emplace_back( eVRSAA );
        }
        m_optionsGFX.m_antialias = ui::Option<OptionsGFX::AntiAlias>{ 0, std::move( aa ), &OptionsGFX::toString<OptionsGFX::AntiAlias> };
    }
    {
        std::pmr::vector<VSync> v{ VSync::eOff, VSync::eOn };
        if ( m_renderer->featureAvailable( Renderer::Feature::eVSyncMailbox ) ) {
            v.emplace_back( VSync::eMailbox );
        }
        m_optionsGFX.m_vsync = ui::Option<VSync>{ 1, std::move( v ), &OptionsGFX::toString<VSync> };
    };

    m_optionsAudio.m_driver.setData( 0, m_audio->listDrivers() );
    m_optionsAudio.m_device.setData( 0, m_audio->listDevices() );

    m_gameUiDataModels.insert( "$data:fullscreen"_hash, &m_optionsGFX.m_fullscreen );
    m_gameUiDataModels.insert( "$data:gammaCorrection"_hash, &m_optionsGFX.m_gamma );
    m_gameUiDataModels.insert( "$data:jet"_hash, &m_optionsCustomize.m_jet );
    m_gameUiDataModels.insert( "$data:missionSelect"_hash, &m_dataMissionSelect );
    m_gameUiDataModels.insert( "$data:resolution"_hash, &m_optionsGFX.m_resolution );
    m_gameUiDataModels.insert( "$data:vsync"_hash, &m_optionsGFX.m_vsync );
    m_gameUiDataModels.insert( "$data:antialias"_hash, &m_optionsGFX.m_antialias );
    m_gameUiDataModels.insert( "$data:fpsLimiter"_hash, &m_optionsGFX.m_fpsLimiter );
    m_gameUiDataModels.insert( "$data:settings.audio.driver"_hash, &m_optionsAudio.m_driver );
    m_gameUiDataModels.insert( "$data:settings.audio.device"_hash, &m_optionsAudio.m_device );
    m_gameUiDataModels.insert( "$data:settings.audio.master"_hash, &m_optionsAudio.m_master );
    m_gameUiDataModels.insert( "$data:settings.audio.ui"_hash, &m_optionsAudio.m_ui );
    m_gameUiDataModels.insert( "$data:settings.audio.sfx"_hash, &m_optionsAudio.m_sfx );
    m_gameUiDataModels.insert( "$data:weaponPrimary"_hash, &m_optionsCustomize.m_weaponPrimary );
    m_gameUiDataModels.insert( "$data:weaponSecondary"_hash, &m_optionsCustomize.m_weaponSecondary );
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
    m_gameCallbacks.insert( "$function:goto_missionBriefing"_hash,  [this](){ changeScreen( Scene::eGameBriefing, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_newgame"_hash, [this](){ changeScreen( Scene::eMissionSelection, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_customize"_hash, [this](){ changeScreen( Scene::eCustomize, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_titlemenu"_hash, [this](){ changeScreen( Scene::eMainMenu, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings"_hash, [this](){ changeScreen( Scene::eSettings, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings_display"_hash, [this](){ changeScreen( Scene::eSettingsDisplay, m_click ); } );
    m_gameCallbacks.insert( "$function:goto_settings_audio"_hash, [this](){ changeScreen( Scene::eSettingsAudio, m_click ); } );
    m_gameCallbacks.insert( "$function:quit"_hash, [this](){ quit(); } );
    m_gameCallbacks.insert( "$function:resume"_hash, [this]{ changeScreen( Scene::eGame, m_click ); } );
    m_gameCallbacks.insert( "$function:applyGFX"_hash, [this]
    {
        DisplayMode displayMode = m_optionsGFX.m_resolution.value();
        displayMode.fullscreen = m_optionsGFX.m_fullscreen.value();
        m_renderer->setVSync( m_optionsGFX.m_vsync.value() );
        m_renderer->setFeatureEnabled( Renderer::Feature::eVRSAA, m_optionsGFX.m_antialias.value() == OptionsGFX::AntiAlias::eVRSAA );
        setDisplayMode( displayMode );
        setTargetFPS( 200, m_optionsGFX.m_fpsLimiter.value() ? FpsLimiter::eSpinLock : FpsLimiter::eOff );
    });
    m_gameCallbacks.insert( "$function:applyAudio"_hash, [this]()
    {
        m_audio->setVolume( Audio::Channel::eMaster, m_optionsAudio.m_master.value() );
        m_audio->setVolume( Audio::Channel::eSFX, m_optionsAudio.m_sfx.value() );
        m_audio->setVolume( Audio::Channel::eUI, m_optionsAudio.m_ui.value() );
        m_audio->selectDriver( m_optionsAudio.m_driver.value() );
        m_audio->selectDevice( m_optionsAudio.m_device.value() );
    } );

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
    m_screens.emplace_back( m_io->viewWait( "ui/mainmenu.ui" ) );

    Targeting::CreateInfo tci{
        .targetSprites{
            "reticle.t.topleft"_hash,
            "reticle.t.topright"_hash,
            "reticle.t.botleft"_hash,
            "reticle.t.botright"_hash,
        },
        .targetSprites2{
            "reticle.t.up"_hash,
            "reticle.t.down"_hash,
            "reticle.t.left"_hash,
            "reticle.t.right"_hash,
        },
        .reticleSprites{
            "reticle.r.topleft"_hash,
            "reticle.r.topright"_hash,
            "reticle.r.botleft"_hash,
            "reticle.r.botright"_hash,
        },
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
    ZoneScoped;
    if ( m_currentScene.load() == Scene::eInit ) [[unlikely]] {
        return;
    }

    const auto [ width, height, aspect ] = viewport();
    const auto [ view, projection ] = getCameraMatrix();
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

    switch ( m_currentScene.load() ) {
    case Scene::eGame:
    case Scene::eGamePaused:
    case Scene::eDead:
    case Scene::eWin:
        renderGameScreen( rctx );
        break;

    case Scene::eMissionSelection:
    case Scene::eCustomize:
    case Scene::eMainMenu:
    case Scene::eSettings:
    case Scene::eSettingsDisplay:
    case Scene::eSettingsAudio:
        renderMenuScreen( rctx, r );
        break;

    default:
        break;
    }

    if ( ui::Screen* screen = currentScreen(); screen ) {
        screen->render( r );
    }

    const PushConstant<Pipeline::eGammaCorrection> pushConstant{
        .m_power = m_optionsGFX.m_gamma.value(),
    };

    const DispatchInfo dispatchInfo{
        .m_pipeline = g_pipelines[ Pipeline::eGammaCorrection ],
    };
    m_renderer->dispatch( dispatchInfo, &pushConstant );
}

void Game::onUpdate( float deltaTime )
{
    ZoneScoped;
    UpdateContext uctx{ .deltaTime = deltaTime, };
    ui::UpdateContext uictx{ .deltaTime = deltaTime, };

    switch ( m_currentScene.load() ) {
    [[unlikely]] case Scene::eInit:
        return;

    case Scene::eGame:
        updateGame( uctx );
        break;
    default:
        m_dustUi.update( uctx );
        break;
    }

    ui::Screen* screen = currentScreen();
    if ( screen ) {
        screen->update( uictx );
    }
}

void Game::pause()
{
    changeScreen( Scene::eGamePaused );
}

void Game::unpause()
{
    changeScreen( Scene::eGame );
}

void Game::updateGame( UpdateContext& updateContext )
{
    ZoneScoped;
    m_player.setInput( m_playerInput );

    if ( m_player.status() == Player::Status::eDead ) {
        changeScreen( Scene::eDead );
    }
    if ( m_enemies.empty() ) {
        changeScreen( Scene::eWin );
    }

    std::pmr::vector<Signal> signals( m_enemies.size() );
    std::transform( m_enemies.begin(), m_enemies.end(), signals.begin(), []( const auto& p ) { return p->signal(); } );
    updateContext.signals = signals;

    m_lookAtTarget.setTarget( m_playerInput.lookAt ? 1.0f : 0.0f );
    m_lookAtTarget.update( updateContext.deltaTime );
    m_player.update( updateContext );
    Bullet::updateAll( updateContext, m_bullets, m_gameScene.explosions(), m_plasma );
    Enemy::updateAll( updateContext, m_enemies );
    m_gameScene.update( updateContext, m_player.position(), m_player.velocity() );
    {
        auto soundsToPlay = m_player.shoot( m_bullets );
        for ( auto&& s : soundsToPlay ) { if ( s ) m_audio->play( s, Audio::Channel::eSFX ); }
        for ( auto&& e : m_enemies ) { e->shoot( m_bullets ); }
    }

    {
        const math::vec3 jetPos = m_player.position();
        auto makeExplosion = [plasma = m_plasma]( const Bullet& b, const math::vec3& p ) -> Explosion
        {
            return Explosion{
                .m_position = p + ( b.m_position - p ) * 15.0_m,
                .m_velocity = b.m_direction * b.m_speed * 0.1f,
                .m_color = color::white,
                .m_texture = plasma,
                .m_size = 16.0_m,
            };
        };

        for ( auto& b : m_bullets ) {
            if ( b.m_type == Bullet::Type::eDead ) continue;
            switch ( b.m_collideId ) {
            case Enemy::COLLIDE_ID:
                if ( !intersectLineSphere( b.m_position, b.m_prevPosition, jetPos, 15.0_m ) ) continue;
                m_player.setDamage( b.m_damage );
                b.m_type = Bullet::Type::eDead;
                m_gameScene.explosions().emplace_back( makeExplosion( b, jetPos ) );
                break;

            case Player::COLLIDE_ID:
                for ( auto& e : m_enemies ) {
                    assert( e );
                    if ( !intersectLineSphere( b.m_position, b.m_prevPosition, e->position(), 15.0_m ) ) continue;
                    e->setDamage( b.m_damage );
                    m_score += b.m_score;
                    b.m_type = Bullet::Type::eDead;
                    m_gameScene.explosions().emplace_back( makeExplosion( b, e->position() ) );
                    break;
                }
                break;
            default:
                assert( !"unreachable" );
                b.m_type = Bullet::Type::eDead;
                break;
            }
        }

        std::erase_if( m_bullets, []( const Bullet& b ) { return b.m_type == Bullet::Type::eDead; } );
    }

    {
        auto isDead = []( const auto& it ) -> bool { return it->status() == SAObject::Status::eDead; };
        for ( auto& e : m_enemies ) {
            if ( isDead( e ) ) {
                m_gameScene.explosions().emplace_back( e->position(), e->velocity(), color::yellowBlaster, m_plasma, 64.0_m, 0.0f );
                continue;
            }
        }
        std::erase_if( m_enemies, isDead );
    }

    m_targeting.setSignals( std::move( signals ) );
    m_targeting.setTarget( m_player.targetSignal(), m_player.targetingState() );
    m_targeting.update( updateContext );
    m_gameplayUIData.m_playerHP = static_cast<float>( m_player.health() ) / 100.0f;
    const math::vec2 reloadState = m_player.reloadState();
    m_gameplayUIData.m_playerReloadPrimary = reloadState.x;
    m_gameplayUIData.m_playerReloadSecondary = reloadState.y;
    m_gameplayUIData.m_jetSpeed = m_player.speed() / 1600_kmph;
    auto [ primary, secondary ] = m_player.weaponClip();
    m_gameplayUIData.m_playerWeaponPrimaryCount = primary;
    m_gameplayUIData.m_playerWeaponSecondaryCount = secondary;

}

void Game::retarget()
{
    if ( m_enemies.empty() ) {
        return;
    }

    struct TgtInfo {
        Signal s;
        float dist;
        bool operator < ( const TgtInfo& rhs ) const noexcept
        {
            return dist < rhs.dist;
        }
    };

    math::vec3 jetPos = m_player.position();
    math::vec3 jetDir = m_player.direction();
    Signal s{};
    auto proc = [f=std::numeric_limits<float>::max(), jetPos, jetDir, &s]( auto& obj ) mutable
    {
        float angle = math::angle( math::normalize( obj->position() - jetPos ), jetDir );
        if ( angle > f ) return;
        f = angle;
        s = obj->signal();
    };
    std::for_each( m_enemies.begin(), m_enemies.end(), std::move( proc ) );
    m_player.setTarget( s );
}

void Game::clearMapData()
{
    ZoneScoped;
    m_enemies.clear();
    m_bullets.clear();
}

void Game::createMapData( const MapCreateInfo& mapInfo, const ModelProto& modelData )
{
    ZoneScoped;
    m_score = 0;
    m_gameScene = GameScene{ mapInfo.texture };

    std::pmr::vector<uint16_t> callsigns( m_callsigns.size() );
    std::iota( callsigns.begin(), callsigns.end(), 0 );
    std::shuffle( callsigns.begin(), callsigns.end(), Random{ std::random_device()() } );
    const auto& w1 = m_weapons[ m_weapon1 ];
    const auto& w2 = m_weapons[ m_weapon2 ];
    m_player = Player( Player::CreateInfo{
        .model = modelData.model,
        .vectorThrust = true,
        .weapons{ w1, w2, w1 },
    } );

    assert( m_enemies.empty() );
    m_enemies.resize( mapInfo.enemies );
    std::generate( m_enemies.begin(), m_enemies.end(), [this, &callsigns, cs=0u]() mutable
    {
        Enemy::CreateInfo ci{
            .model = &m_enemyModel,
            .callsign = callsigns[ cs++ ],
        };
        UniquePointer<Enemy> ptr{ &m_poolEnemies, ci };
        ptr->setTarget( &m_player );
        ptr->setWeapon( m_enemyWeapon );
        return ptr;
    });
    m_player.setTarget( m_enemies.front()->signal() );
    m_gameplayUIData.m_playerWeaponIconPrimary = w1.displayIcon;
    m_gameplayUIData.m_playerWeaponIconSecondary = w2.displayIcon;
}

void Game::changeScreen( Scene scene, Audio::Slot sound )
{
    ZoneScoped;
    if ( sound ) {
        m_audio->play( sound, Audio::Channel::eUI );
    }
    SDL_ShowCursor( scene != Scene::eGame );
    auto setScreen = [this]( auto hash )
    {
        auto it = std::ranges::find_if( m_screens, [hash]( const auto& sc ) { return sc.name() == hash; } );
        assert( it != m_screens.end() );
        m_currentScreen = &*it;
    };

    m_currentScene = scene;
    switch ( scene ) {
    case Scene::eMainMenu: setScreen( "mainMenu"_hash ); break;
    case Scene::eCustomize: setScreen( "customize"_hash ); break;
    case Scene::eSettings: setScreen( "settings"_hash ); break;
    case Scene::eSettingsDisplay: setScreen( "settings.display"_hash ); break;
    case Scene::eSettingsAudio: setScreen( "settings.audio"_hash ); break;
    case Scene::eGame: setScreen( "gameplay"_hash ); break;
    case Scene::eGamePaused: setScreen( "pause"_hash ); break;
    case Scene::eDead:
        m_uiMissionResult = g_uiProperty.localize( "missionLost"_hash );
        m_uiMissionScore = g_uiProperty.localize( "yourScore"_hash ) + intToUTF32( m_score );
        setScreen( "result"_hash );
        break;

    case Scene::eWin:
        m_uiMissionResult = g_uiProperty.localize( "missionWin"_hash );
        m_uiMissionScore = g_uiProperty.localize( "yourScore"_hash ) + intToUTF32( m_score );
        setScreen( "result"_hash );
        break;

    case Scene::eGameBriefing:
        createMapData( m_mapsContainer[ m_currentMission ], m_jetsContainer[ m_currentJet ] );
        changeScreen( Scene::eGamePaused );
        break;

    case Scene::eMissionSelection:
        clearMapData();
        setScreen( "missionSelect"_hash );
        break;

    default:
        assert( !"unhandled enum" );
        break;
    }

    if ( ui::Screen* screen = currentScreen() ) {
        auto [ w, h, a ] = viewport();
        screen->show( { w, h } );
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
    auto loc = cfg::Entry::fromData( asset.data );
    Hash hash{};
    for ( const auto& it : loc ) {
        m_localizationMap.insert( hash( *it ), it.toString32() );
    }
    g_uiProperty.m_locTable = m_localizationMap.makeView();
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
    case GameAction::eJetPitch: m_playerInput.pitch = -a.analog(); break;
    case GameAction::eJetYaw: m_playerInput.yaw = -a.analog(); break;
    case GameAction::eJetRoll: m_playerInput.roll = a.analog(); break;
    case GameAction::eJetShoot1: m_playerInput.shoot1 = a.digital(); break;
    case GameAction::eJetShoot2: m_playerInput.shoot2 = a.digital(); break;
    case GameAction::eJetSpeed: m_playerInput.speed = a.analog(); break;
    case GameAction::eJetLookAt: m_playerInput.lookAt = a.digital(); break;
    default: break;
    }

    do {
        ui::Screen* screen = currentScreen();
        if ( !screen ) break;
        if ( !a.testEnumRange<(Action::Enum)ui::Action::base, (Action::Enum)ui::Action::end>() ) break;
        screen->onAction( ui::Action{ .a = a.toA<ui::Action::Enum>(), .value = a.value } );
    } while ( 0 );

    if ( m_currentScene.load() == Scene::eGame ) {
        switch ( action ) {
        case GameAction::eJetTarget:
            if ( a.digital() ) { retarget(); }
            return;
        case GameAction::eGamePause:
            if ( a.digital() ) { pause(); }
            return;
        default:
            return;
        }
    }
}

void Game::onMouseEvent( const MouseEvent& mouseEvent )
{
    ZoneScoped;
    switch ( mouseEvent.type ) {
    case MouseEvent::eClickSecondary:
        onAction( Action{ .userEnum = (::Action::Enum)ui::Action::eMenuCancel, .value = mouseEvent.value } );
        return;
    case MouseEvent::eClickMiddle:
        return;
    case MouseEvent::eClick:
        if ( mouseEvent.value == 0 ) return;
    case MouseEvent::eMove: break;
    }
    ui::Screen* screen = currentScreen();
    if ( screen ) {
        screen->onMouseEvent( mouseEvent );
    }
}

void Game::renderGameScreen( RenderContext rctx )
{
    render3D( rctx );
    m_targeting.render( rctx );
}

std::tuple<math::vec3, math::vec3, math::vec3> Game::getCamera() const
{
    math::vec3 jetPos = m_player.position();
    math::vec3 jetCamPos = jetPos + m_player.cameraPosition() * m_player.rotation();
    math::vec3 jetCamUp = math::vec3{ 0, 1, 0 } * m_player.rotation();
    math::vec3 jetCamTgt = jetCamPos + m_player.cameraDirection();

    Signal tgt = m_player.targetSignal();
    math::vec3 lookAtTgt = tgt ? tgt.position : jetCamTgt;
    math::vec3 lookAtPos = math::vec3{ 0.0f, -20.0_m, 0.0f } * m_player.rotation() + jetPos - math::normalize( lookAtTgt - jetPos ) * 42.8_m;

    math::vec3 retPos = math::lerp( jetCamPos, lookAtPos, m_lookAtTarget.value() );
    math::vec3 retTgt = math::lerp( jetCamTgt, lookAtTgt, m_lookAtTarget.value() );

    return { retPos, jetCamUp, retTgt };
}

std::tuple<math::mat4, math::mat4> Game::getCameraMatrix() const
{
    const auto [ cameraPos, cameraUp, cameraTgt ] = getCamera();
    return {
        math::lookAt( cameraPos, cameraTgt, cameraUp ),
        math::perspective( math::radians( 55.0f + m_player.speed() * 3 ), viewportAspect(), 0.001f, 2000.0f )
    };
}

void Game::render3D( RenderContext rctx )
{
    std::tie( rctx.view, rctx.projection ) = getCameraMatrix();
    std::tie( rctx.cameraPosition, rctx.cameraUp, std::ignore ) = getCamera();

    m_gameScene.render( rctx );
    Enemy::renderAll( rctx, m_enemies );
    Bullet::renderAll( rctx, m_bullets, m_plasma );
    m_player.render( rctx );

    switch ( m_optionsGFX.m_antialias.value() ) {
    case OptionsGFX::AntiAlias::eFXAA:
    case OptionsGFX::AntiAlias::eVRSAA: {
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

    switch ( m_optionsGFX.m_antialias.value() ) {
    case OptionsGFX::AntiAlias::eFXAA:
    case OptionsGFX::AntiAlias::eVRSAA: {
        const PushConstant<Pipeline::eAntiAliasFXAA> aa{};
        const DispatchInfo daa{ .m_pipeline = g_pipelines[ Pipeline::eAntiAliasFXAA ] };
        rctx.renderer->dispatch( daa, &aa );
    } break;
    default:
        break;
    }
}

void Game::onActuator( Actuator a )
{
    bool inputChanged = false;
    switch ( a.source ) {
    case Actuator::Source::eKBM:
        inputChanged = g_uiProperty.setInputSource( ui::InputSource::eKBM );
        break;
    case Actuator::Source::eXBoxOne:
        inputChanged = g_uiProperty.setInputSource( ui::InputSource::eXBoxOne );
        break;
    default:
        assert( !"unhandled enum" );
        break;
    }
    if ( ui::Screen* screen = currentScreen(); inputChanged && screen ) {
        screen->refreshInput();
    }

    const std::pmr::vector<Action> actions = m_actionStateTracker.updateAndResolve( a );
    for ( const auto& it : actions ) {
        onAction( it );
    }
}
