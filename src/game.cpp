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

#include <Tracy.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <cmath>
#include <set>

static constexpr const char* chunk1[] = {
    "textures/a2.dds",
    "textures/a3.dds",
    "textures/a4.dds",
    "textures/a5.dds",
    "textures/plasma.dds",
};

constexpr std::tuple<GameAction, Actuator> inputActions[] = {
    { GameAction::eGamePause, SDL_CONTROLLER_BUTTON_START },
    { GameAction::eGamePause, SDL_SCANCODE_ESCAPE },
    { GameAction::eJetPitch, SDL_CONTROLLER_AXIS_LEFTY },
    { GameAction::eJetRoll, SDL_CONTROLLER_AXIS_RIGHTX },
    { GameAction::eJetShoot1, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { GameAction::eJetShoot1, SDL_SCANCODE_J },
    { GameAction::eJetShoot2, SDL_CONTROLLER_BUTTON_X },
    { GameAction::eJetShoot2, SDL_SCANCODE_K },
    { GameAction::eJetShoot3, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
    { GameAction::eJetShoot3, SDL_SCANCODE_L },
    { GameAction::eJetTarget, SDL_CONTROLLER_BUTTON_Y },
    { GameAction::eJetLookAt, SDL_CONTROLLER_BUTTON_LEFTSHOULDER },
    { GameAction::eJetTarget, SDL_SCANCODE_I },
    { GameAction::eJetYaw, SDL_CONTROLLER_AXIS_LEFTX },
};

constexpr std::tuple<ui::Action::Enum, Actuator> UI_INPUT[] = {
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


static std::pmr::vector<ModelProto> loadJets( std::span<const uint8_t> data )
{
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( data );

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

static std::pmr::vector<MapCreateInfo> loadMaps( std::span<const uint8_t> data )
{
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( data );

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

static ui::Atlas loadUIAtlas( std::span<const uint8_t> span )
{
    ZoneScoped;
    cfg::Entry entry = cfg::Entry::fromData( span );
    uint16_t width = 0;
    uint16_t height = 0;
    Hash hash{};
    auto gibSprite = []( const auto& entry )
    {
        Hash hash{};
        ui::Atlas::Sprite sprite{};
        for ( auto&& it : entry ) {
            switch ( hash( *it ) ) {
            case "x"_hash: sprite.x = it.template toInt<uint16_t>(); continue;
            case "y"_hash: sprite.y = it.template toInt<uint16_t>(); continue;
            case "w"_hash: sprite.w = it.template toInt<uint16_t>(); continue;
            case "h"_hash: sprite.h = it.template toInt<uint16_t>(); continue;
            default:
                assert( !"unhandled property" );
                continue;
            }
        }
        assert( sprite.w );
        assert( sprite.h );
        return sprite;
    };

    std::pmr::vector<ui::Atlas::SpritePack> sprites{};
    for ( auto&& it : entry ) {
        switch ( auto h = hash( *it ) ) {
        case "width"_hash: width = it.template toInt<uint16_t>(); continue;
        case "height"_hash: height = it.template toInt<uint16_t>(); continue;
        default:
            sprites.emplace_back( std::make_tuple( h, gibSprite( it ) ) );
            continue;
        }
    }
    assert( !sprites.empty() );
    assert( width );
    assert( height );
    std::sort( sprites.begin(), sprites.end(), &ui::Atlas::sortCmp );
    return ui::Atlas{ sprites, width, height };
}

static std::tuple<WeaponCreateInfo, bool> parseWeapon( const cfg::Entry& entry )
{
    using std::literals::string_view_literals::operator""sv;
    auto makeType = []( std::string_view sv )
    {
        Hash hash{};
        switch ( hash( sv ) ) {
        default:
            assert( !"unknown type" );
            [[fallthrough]];
        case "blaster"_hash: return Bullet::Type::eBlaster;
        case "torpedo"_hash: return Bullet::Type::eTorpedo;
        }
    };

    auto makeColor = []( std::string_view sv )
    {
        Hash hash{};
        switch ( hash( sv ) ) {
        case "blaster"_hash: return color::blaster;
        case "dodgerBlue"_hash: return color::dodgerBlue;
        case "orchid"_hash: return color::orchid;
        case "red"_hash: return color::crimson;
        case "white"_hash: return color::white;
        case "yellow"_hash: return color::yellow;
        case "yellowBlaster"_hash: return color::yellowBlaster;
        default:
            assert( !"unknown color" );
            return color::orchid;
        }
    };

    Hash hash{};
    WeaponCreateInfo weap{};
    bool isHidden = false;
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "color1"_hash: weap.color1 = makeColor( property.toString() ); continue;
        case "color2"_hash: weap.color2 = makeColor( property.toString() ); continue;
        case "damage"_hash: weap.damage = property.toInt<uint8_t>(); continue;
        case "delay"_hash: weap.delay = property.toFloat(); continue;
        case "hidden"_hash: isHidden = property.toInt<bool>(); continue;
        case "kmph"_hash: weap.speed = property.toFloat() * (float)kmph; continue;
        case "loc"_hash: weap.displayName = hash( property.toString() ); continue;
        case "score"_hash: weap.score_per_hit = property.toInt<uint16_t>(); continue;
        case "size"_hash: weap.size = property.toFloat() * (float)meter; continue;
        case "type"_hash: weap.type = makeType( property.toString() ); continue;
        case "sound"_hash: continue; // TODO
        case "texture"_hash: continue; // TODO
        default:
            assert( !"unknown weapon property" );
            continue;
        }
    }
    return { weap, isHidden };
}

Game::Game( int argc, char** argv )
: Engine{ argc, argv }
{
    ZoneScoped;
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
    m_glow.setSize( { w, h } );
    if ( auto* screen = currentScreen() ) {
        screen->resize( { w, h } );
    }
    m_hud.resize( { w, h } );
    m_uiRings.setSize( { w, h } );
}

void Game::onInit()
{
    ZoneScoped;
    m_io->mount( "shaders.tar" );
    m_io->mount( "fonts.tar" );
    m_io->mount( "misc.tar" );
    m_io->mount( "ui.tar" );
    m_io->mount( "sounds.tar" );
    m_io->mount( "models.tar" );
    m_io->mount( "textures.tar" );


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
    g_uiProperty.m_pipelineSpriteSequenceRGBA = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE_RGBA );
    g_uiProperty.m_pipelineSpriteSequenceColors = setupPipeline( m_renderer, m_io, ui::SPRITE_SEQUENCE_COLORS );

    m_enemies.reserve( 100 );
    m_explosions.reserve( 100 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
    m_jetsContainer.reserve( 5 );
    m_mapsContainer.reserve( 5 );
    for ( auto [ eid, act ] : inputActions ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), act );
    }
    for ( auto [ eid, act ] : UI_INPUT ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), act );
    }
    for ( auto [ eid, min, max ] : inputActions2 ) {
        m_actionStateTracker.add( static_cast<Action::Enum>( eid ), min, max );
    }

    {
        ZoneScopedN( "HashMap" );
        auto loc = cfg::Entry::fromData( m_io->viewWait( "lang/en.txt" ) );
        Hash hash{};
        for ( const auto& it : loc ) {
            m_localizationMap.insert( hash( *it ), it.toString32() );
        }
        g_uiProperty.m_locTable = m_localizationMap.makeView();
    }

    m_uiAtlas = loadUIAtlas( m_io->viewWait( "misc/ui_atlas.txt" ) );
    m_dustUi.setVelocity(  math::vec3{ 0.0f, 0.0f, 26.0_m }  );
    m_dustUi.setCenter( {} );
    m_dustUi.setLineWidth( 2.0f );

    g_gameCallbacks[ "$function:goto_missionBriefing" ] = [this](){ changeScreen( Screen::eGameBriefing, m_click ); };
    g_gameCallbacks[ "$function:goto_newgame" ] = [this](){ changeScreen( Screen::eMissionSelection, m_click ); };
    g_gameCallbacks[ "$function:goto_customize" ] = [this](){ changeScreen( Screen::eCustomize, m_click ); };
    g_gameCallbacks[ "$function:goto_titlemenu" ] = [this](){ changeScreen( Screen::eMainMenu, m_click ); };
    g_gameCallbacks[ "$function:goto_settings" ] = [this](){ changeScreen( Screen::eSettings, m_click ); };
    g_gameCallbacks[ "$function:goto_settings_display" ] = [this](){ changeScreen( Screen::eSettingsDisplay, m_click ); };
    g_gameCallbacks[ "$function:quit" ] = [this](){ quit(); };
    g_gameCallbacks[ "$function:resume" ] = [this]{ changeScreen( Screen::eGame, m_click ); };
    g_gameCallbacks[ "$function:applyGFX" ] = [this]
    {
        DisplayMode displayMode = m_optionsGFX.m_resolution.value();
        displayMode.fullscreen = m_optionsGFX.m_fullscreen.value();
        setDisplayMode( displayMode );
        m_renderer->setVSync( m_optionsGFX.m_vsync.value() );
    };

    g_uiProperty.m_colorA = color::dodgerBlue;

    g_gameUiDataModels[ "$data:gammaCorrection" ] = &m_optionsGFX.m_gamma;

    m_optionsGFX.m_resolution = ui::Option<DisplayMode>{ 0, displayModes(),
        []( const auto& dm ) { return intToUTF32( dm.width ) + U" x " + intToUTF32( dm.height ); }
    };

    {
        std::pmr::vector<VSync> v{ VSync::eOff, VSync::eOn };
        std::pmr::vector<Hash::value_type> h{ "off"_hash, "on"_hash };
        if ( m_renderer->supportedVSync( VSync::eMailbox ) ) {
            v.emplace_back( VSync::eMailbox );
            h.emplace_back( "mailbox"_hash );
        }
        m_optionsGFX.m_vsync = ui::Option<VSync>{ 1, std::move( v ), std::move( h ) };
    };

    g_gameUiDataModels[ "$data:resolution" ] = &m_optionsGFX.m_resolution;
    g_gameUiDataModels[ "$data:fullscreen" ] = &m_optionsGFX.m_fullscreen;
    g_gameUiDataModels[ "$data:vsync" ] = &m_optionsGFX.m_vsync;

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
    g_gameUiDataModels[ "$data:jet" ] = &m_optionsCustomize.m_jet;

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
    m_optionsCustomize.m_weaponSecondary.m_size = weaponCount;
    m_optionsCustomize.m_weaponSecondary.m_at = weapNames;
    m_optionsCustomize.m_weaponSecondary.m_select = [this]( auto i ){ m_weapon2 = i; };
    m_optionsCustomize.m_weaponSecondary.m_current = [this](){ return m_weapon2; };
    g_gameUiDataModels[ "$data:weaponPrimary" ] = &m_optionsCustomize.m_weaponPrimary;
    g_gameUiDataModels[ "$data:weaponSecondary" ] = &m_optionsCustomize.m_weaponSecondary;


    m_dataMissionSelect.m_current = [this](){ return m_currentMission; };
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
    g_gameUiDataModels[ "$data:missionSelect" ] = &m_dataMissionSelect;

    {
        ui::Font::CreateInfo dejavu12{
            .fontAtlas = m_io->viewWait( "fonts/dejavu_24.fnta" ),
            .texture = parseTexture( m_io->viewWait( "fonts/dejavu_24.dds" ) ),
            .lineHeight = 12,
        };
        ui::Font::CreateInfo dejavu18{
            .fontAtlas = m_io->viewWait( "fonts/dejavu_36.fnta" ),
            .texture = parseTexture( m_io->viewWait( "fonts/dejavu_36.dds" ) ),
            .lineHeight = 18,
        };
        ui::Font::CreateInfo dejavu32{
            .fontAtlas = m_io->viewWait( "fonts/dejavu_64.fnta" ),
            .texture = parseTexture( m_io->viewWait( "fonts/dejavu_64.dds" ) ),
            .lineHeight = 32,
        };
        auto* alloc = std::pmr::get_default_resource();
        m_fontSmall = UniquePointer<ui::Font>{ alloc, dejavu12 };
        m_fontMedium = UniquePointer<ui::Font>{ alloc, dejavu18 };
        m_fontLarge = UniquePointer<ui::Font>{ alloc, dejavu32 };
        g_uiProperty.m_fontSmall = m_fontSmall.get();
        g_uiProperty.m_fontMedium = m_fontMedium.get();
        g_uiProperty.m_fontLarge = m_fontLarge.get();
    }

    m_hud = Hud{ &m_hudData };

    m_blaster = m_audio->load( m_io->viewWait( "sounds/blaster.wav" ) );
    m_torpedo = m_audio->load( m_io->viewWait( "sounds/torpedo.wav" ) );
    m_click = m_audio->load( m_io->viewWait( "sounds/click.wav" ) );

    const std::array rings = {
        parseTexture( m_io->viewWait( "textures/cyber_ring1.dds" ) ),
        parseTexture( m_io->viewWait( "textures/cyber_ring2.dds" ) ),
        parseTexture( m_io->viewWait( "textures/cyber_ring3.dds" ) ),
    };
    m_uiRings = UIRings{ rings };

    m_textures[ "textures/atlas_ui.dds" ] = parseTexture( m_io->viewWait( "textures/atlas_ui.dds" ) );
    g_uiProperty.m_atlasTexture = m_textures[ "textures/atlas_ui.dds" ];
    g_uiProperty.m_atlas = &m_uiAtlas;


    for ( const char* it : chunk1 ) {
        m_textures[ it ] = parseTexture( m_io->viewWait( it ) );
    }

    m_plasma = m_textures[ "textures/plasma.dds" ];

    cfg::Entry weapons = cfg::Entry::fromData( m_io->viewWait( "misc/weapons.cfg" ) );
    for ( const auto& it : weapons ) {
        auto [ weapon, isHidden ] = parseWeapon( it );
        if ( isHidden ) {
            m_enemyWeapon = weapon;
        }
        else {
            m_weapons.emplace_back( weapon );
        }
    }
    loadMapProto();

    m_jetsContainer = loadJets( m_io->viewWait( "misc/jets.cfg" ) );
    assert( !m_jetsContainer.empty() );
    for ( auto& it : m_jetsContainer ) {
        m_meshes[ it.model_file ] = Mesh{ m_io->viewWait( it.model_file ), m_renderer };
        it.model = Model( m_meshes[ it.model_file ], m_textures[ it.model_texture ], it.scale );
    }

    m_screenCustomize =     cfg::Entry::fromData( m_io->viewWait( "ui/customize.ui" ) );
    m_screenGameplay =      cfg::Entry::fromData( m_io->viewWait( "ui/gameplay.ui" ) );
    m_screenMissionResult = cfg::Entry::fromData( m_io->viewWait( "ui/result.ui" ) );
    m_screenMissionSelect = cfg::Entry::fromData( m_io->viewWait( "ui/missionselect.ui" ) );
    m_screenPause =         cfg::Entry::fromData( m_io->viewWait( "ui/pause.ui" ) );
    m_screenSettings =      cfg::Entry::fromData( m_io->viewWait( "ui/settings.ui" ) );
    m_screenSettingsDisplay = cfg::Entry::fromData( m_io->viewWait( "ui/settings_display.ui" ) );
    m_screenTitle =         cfg::Entry::fromData( m_io->viewWait( "ui/mainmenu.ui" ) );

    m_enemyModel = Model{ m_meshes[ "models/a2.objc" ], m_textures[ "textures/a2.dds" ], 0.45f };

    onResize( viewportWidth(), viewportHeight() );

    changeScreen( Screen::eMainMenu );
}

ui::Screen* Game::currentScreen()
{
    switch ( m_currentScreen ) {
    case Screen::eGame:
        return &m_screenGameplay;

    case Screen::eGamePaused:
        return &m_screenPause;

    case Screen::eDead:
    case Screen::eWin:
        return &m_screenMissionResult;

    case Screen::eMissionSelection:
        return &m_screenMissionSelect;

    case Screen::eMainMenu:
        return &m_screenTitle;

    case Screen::eCustomize:
        return &m_screenCustomize;

    case Screen::eSettings:
        return &m_screenSettings;

    case Screen::eSettingsDisplay:
        return &m_screenSettingsDisplay;

    default:
        return nullptr;
    }
}

void Game::onRender( RenderContext rctx )
{
    ZoneScoped;
    if ( m_currentScreen.load() == Screen::eInit ) [[unlikely]] {
        return;
    }

    const auto [ width, height, aspect ] = viewport();
    const auto [ view, projection ] = getCameraMatrix();
    rctx.projection = math::ortho( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ), -100.0f, 100.0f );
    rctx.camera3d = projection * view;
    rctx.viewport = { width, height };

    const ui::RenderContext r{
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
    case Screen::eDead:
    case Screen::eWin:
        renderGameScreen( rctx, r );
        m_uiRings.render( r );
        m_glow.render( r );
        break;

    case Screen::eMissionSelection:
    case Screen::eCustomize:
    case Screen::eMainMenu:
    case Screen::eSettings:
    case Screen::eSettingsDisplay:
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

    const PushConstant<Pipeline::eScanline> pushScanline{ .m_power = { 0.816f, 0.816f, 0.816f, 1.0f } };
    const DispatchInfo dispatchScanline{ .m_pipeline = g_pipelines[ Pipeline::eScanline ] };
    m_renderer->dispatch( dispatchScanline, &pushScanline );
}

void Game::onUpdate( const UpdateContext& uctx )
{
    ZoneScoped;

    switch ( m_currentScreen ) {
    [[unlikely]] case Screen::eInit:
        return;

    case Screen::eGame:
        updateGame( uctx );
        break;
    default:
        m_dustUi.update( uctx );
        m_uiRings.update( uctx );
        break;
    }

    ui::Screen* screen = currentScreen();
    if ( screen ) {
        screen->update( uctx );
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

    auto added = m_jet.shoot( &m_poolBullets, &m_bullets );
    playSounds( added );

    m_lookAtTarget.setTarget( m_jetInput.lookAt ? 1.0f : 0.0f );
    m_lookAtTarget.update( updateContext.deltaTime );
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
        auto bulletsToRemove = std::remove_if( m_bullets.begin(), m_bullets.end(), isDead );
        auto makeExplosion = [plasma = m_plasma]( const auto& ptr ) -> Explosion
        {
            assert( ptr );
            return Explosion{
                .m_position = ptr->position(),
                .m_velocity = ptr->velocity() * 0.1f,
                .m_color = ptr->color(),
                .m_texture = plasma,
                .m_size = 16.0_m,
            };
        };
        std::transform( bulletsToRemove, m_bullets.end(), std::back_inserter( m_explosions ), makeExplosion );
        m_bullets.erase( bulletsToRemove, m_bullets.end() );
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
                m_jet.untarget( e.get() );
                m_explosions.push_back( Explosion{ e->position(), e->velocity(), color::yellowBlaster, m_plasma } );
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

    m_targeting.update( updateContext );
    m_targeting.setState( m_jet.targetingState() );
    m_hudData.calc = static_cast<uint32_t>( m_fpsMeter.calculated() );
    m_hudData.fps = static_cast<uint32_t>( m_fpsMeter.fps() );
    m_hudData.pool = static_cast<uint32_t>( m_poolBullets.allocCount() );
    m_hudData.speed = m_jet.speed();
    m_uiPlayerHP = static_cast<float>( m_jet.health() ) / 100.0f;
    m_hud.update( updateContext );
}

void Game::playSounds( std::span<UniquePointer<Bullet>> bullets )
{
    ZoneScoped;
    assert( m_audio );
    m_hudData.shots += bullets.size();
    for ( const auto& it : bullets ) {
        switch ( it->type() ) {
        case Bullet::Type::eBlaster: m_audio->play( m_blaster ); break;
        case Bullet::Type::eTorpedo: m_audio->play( m_torpedo ); break;
        }
    }
}

void Game::retarget()
{
    if ( m_enemies.empty() ) {
        return;
    }

    struct TgtInfo {
        SAObject* obj;
        float dist;
        bool operator < ( const TgtInfo& rhs ) const noexcept
        {
            return dist < rhs.dist;
        }
    };

    math::vec3 jetPos = m_jet.position();
    math::vec3 jetDir = m_jet.direction();
    std::pmr::vector<TgtInfo> tgtInfo;
    tgtInfo.resize( m_enemies.size() );
    std::transform( m_enemies.begin(), m_enemies.end(), tgtInfo.begin(),
        [jetPos, jetDir]( auto& obj ) -> TgtInfo
        {
            return { obj.get(), math::angle( math::normalize( obj->position() - jetPos ), jetDir ) };
        }
    );
    auto it = std::min_element( tgtInfo.begin(), tgtInfo.end() );

    m_jet.setTarget( it->obj );
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
    };
    m_skybox = Skybox{ mapInfo.texture };
    m_explosions.clear();
    m_dustGame.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_dustGame.setCenter( {} );
    m_dustGame.setLineWidth( 2.0f );

    const auto& w1 = m_weapons[ m_weapon1 ];
    const auto& w2 = m_weapons[ m_weapon2 ];
    m_jet = Jet( Jet::CreateInfo{
        .model = modelData.model,
        .modelScale = modelData.scale,
        .vectorThrust = true,
        .weapons{ w1, w2, w1 },
    } );

    assert( m_enemies.empty() );
    m_enemies.resize( mapInfo.enemies );
    std::generate( m_enemies.begin(), m_enemies.end(), [this]()
    {
        UniquePointer<Enemy> ptr{ &m_poolEnemies, &m_enemyModel };
        ptr->setTarget( &m_jet );
        ptr->setWeapon( m_enemyWeapon );
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

    switch ( scr ) {
    case Screen::eMainMenu:
    case Screen::eCustomize:
    case Screen::eSettings:
    case Screen::eSettingsDisplay:
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
        m_uiMissionResult = g_uiProperty.localize( "missionLost"_hash );
        m_uiMissionScore = g_uiProperty.localize( "yourScore"_hash ) + intToUTF32( m_hudData.score );
        m_currentScreen = scr;
        break;

    case Screen::eWin:
        m_uiMissionResult = g_uiProperty.localize( "missionWin"_hash );
        m_uiMissionScore = g_uiProperty.localize( "yourScore"_hash ) + intToUTF32( m_hudData.score );
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

    if ( ui::Screen* screen = currentScreen() ) {
        auto [ w, h, a ] = viewport();
        screen->show( { w, h } );
    }
}

void Game::loadMapProto()
{
    ZoneScoped;
    assert( m_mapsContainer.empty() );
    m_mapsContainer = loadMaps( m_io->viewWait( "misc/maps.cfg" ) );
    assert( !m_mapsContainer.empty() );

    std::pmr::set<std::filesystem::path> uniqueTextures;
    for ( auto it : m_mapsContainer ) {
        uniqueTextures.insert( it.previewPath );
        for ( auto p : it.filePath ) {
            uniqueTextures.insert( p );
        }
    }

    for ( const auto& it : uniqueTextures ) {
        m_textures[ it ] = parseTexture( m_io->viewWait( it ) );
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
    case GameAction::eJetPitch: m_jetInput.pitch = -a.analog(); break;
    case GameAction::eJetYaw: m_jetInput.yaw = -a.analog(); break;
    case GameAction::eJetRoll: m_jetInput.roll = a.analog(); break;
    case GameAction::eJetShoot1: m_jetInput.shoot1 = a.digital(); break;
    case GameAction::eJetShoot2: m_jetInput.shoot2 = a.digital(); break;
    case GameAction::eJetShoot3: m_jetInput.shoot3 = a.digital(); break;
    case GameAction::eJetSpeed: m_jetInput.speed = a.analog(); break;
    case GameAction::eJetLookAt: m_jetInput.lookAt = a.digital(); break;
    default: break;
    }

    ui::Screen* screen = currentScreen();
    do {
        if ( !screen ) break;
        if ( !a.digital() ) break;
        if ( !a.testEnumRange<(Action::Enum)ui::Action::base, (Action::Enum)ui::Action::end>() ) break;
        screen->onAction( ui::Action{ .a = a.toA<ui::Action::Enum>(), .value = 1 } );
    } while ( 0 );

    if ( m_currentScreen == Screen::eGame ) {
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
    ui::Screen* screen = currentScreen();
    if ( screen ) {
        screen->onMouseEvent( mouseEvent );
    }
}

void Game::renderGameScreen( RenderContext rctx, ui::RenderContext r )
{
    render3D( rctx );
    m_hud.render( r );
    m_targeting.render( rctx );
}

std::tuple<math::vec3, math::vec3, math::vec3> Game::getCamera() const
{
    math::vec3 jetPos = m_jet.position();
    math::vec3 jetCamPos = jetPos + m_jet.cameraPosition() * m_jet.rotation();
    math::vec3 jetCamUp = math::vec3{ 0, 1, 0 } * m_jet.rotation();
    math::vec3 jetCamTgt = jetCamPos + m_jet.cameraDirection();

    math::vec3 lookAtTgt = m_targeting.target() ? *m_targeting.target() : jetCamTgt;
    math::vec3 lookAtPos = math::vec3{ 0.0f, -20.0_m, 0.0f } * m_jet.rotation() + jetPos - math::normalize( lookAtTgt - jetPos ) * 42.8_m;

    math::vec3 retPos = math::nonlerp( jetCamPos, lookAtPos, m_lookAtTarget.value() );
    math::vec3 retTgt = math::nonlerp( jetCamTgt, lookAtTgt, m_lookAtTarget.value() );

    return { retPos, jetCamUp, retTgt };
}

std::tuple<math::mat4, math::mat4> Game::getCameraMatrix() const
{
    const auto [ cameraPos, cameraUp, cameraTgt ] = getCamera();
    return {
        math::lookAt( cameraPos, cameraTgt, cameraUp ),
        math::perspective( math::radians( 55.0f + m_jet.speed() * 3 ), viewportAspect(), 0.001f, 2000.0f )
    };
}

void Game::render3D( RenderContext rctx )
{
    std::tie( rctx.view, rctx.projection ) = getCameraMatrix();
    std::tie( rctx.cameraPosition, rctx.cameraUp, std::ignore ) = getCamera();

    m_skybox.render( rctx );
    Explosion::renderAll( rctx, m_explosions, m_plasma );
    m_dustGame.render( rctx );
    Enemy::renderAll( rctx, m_enemies );
    Bullet::renderAll( rctx, m_bullets, m_plasma );
    Bullet::renderAll( rctx, m_enemyBullets, m_plasma );
    m_jet.render( rctx );
}

void Game::renderBackground( ui::RenderContext rctx ) const
{
    [[maybe_unused]]
    const auto [ w, h, a ] = viewport();
    const math::vec2 uv = math::vec2{ w, h } / m_uiAtlas.extent();

    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eBackground ],
        .m_verticeCount = 4,
    };
    pushBuffer.m_resource[ 1 ].texture = g_uiProperty.atlasTexture();

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

    m_uiRings.render( r );
    m_glow.render( r );
}

void Game::onActuator( Actuator a )
{
    const std::pmr::vector<Action> actions = m_actionStateTracker.updateAndResolve( a );
    for ( const auto& it : actions ) {
        onAction( it );
    }
}
