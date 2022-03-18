#include "game.hpp"

#include "colors.hpp"
#include "constants.hpp"
#include "game_action.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"

#include <shared/cfg.hpp>

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
    enum AST {
        eName,
        eIdentifier,
        eIdentifierOrPop,
        eValue,
        eAssignValue,
        eAssignPush,
        ePush,
    };
    AST expectedTokens = eName;

    enum AssignTo {
        eNone,
        eModel,
        eTexture,
        eScale,
    };
    AssignTo assignTo = eNone;

    ModelProto mod;
    std::pmr::vector<ModelProto> jets;
    jets.reserve( 4 );

    using std::literals::string_view_literals::operator""sv;
    auto whitespace = U" \t\n\r"sv;

    cfg::TokenIterator it{ cfg::c_separators, str };
    for ( auto token = *it; ( token = *it ); ++it ) {
        if ( whitespace.find( static_cast<char32_t>( token.userEnum ) ) != std::u32string_view::npos ) { continue; }
        switch ( expectedTokens ) {
        case eName:
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            mod.name = std::u32string{ token.data, token.data + token.length }; // TODO utf-32
            expectedTokens = eAssignPush;
            continue;

        case eAssignPush:
            assert( token.userEnum == '=' );
            expectedTokens = ePush;
            continue;

        case ePush:
            assert( token.userEnum == '{' );
            expectedTokens = eIdentifier;
            continue;

        case eIdentifierOrPop:
            if ( token.userEnum == '}' ) {
                jets.push_back( mod );
                mod = {};
                expectedTokens = eName;
                continue;
            }
            [[fallthrough]];
        case eIdentifier: {
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            std::string_view v = *token;
            expectedTokens = eAssignValue;
            if ( v == "model"sv ) { assignTo = eModel; continue; }
            if ( v == "texture"sv ) { assignTo = eTexture; continue; };
            if ( v == "scale"sv ) { assignTo = eScale; continue; };
            assert( !"unhandled identifier" );
            continue;
        }

        case eAssignValue:
            assert( token.userEnum == '=' );
            expectedTokens = eValue;
            continue;

        case eValue:
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            switch ( assignTo ) {
            case eModel: mod.model_file = *token; break;
            case eTexture: mod.model_texture = *token; break;
            case eScale: mod.scale = std::strtof( token.data, nullptr ); break;
            default: assert( !"unhandled variable" );
            }
            assignTo = eNone;
            expectedTokens = eIdentifierOrPop;
            continue;
        }

    }
    assert( expectedTokens == eName );
    assert( assignTo == eNone );

    return jets;
}

static std::pmr::vector<MapCreateInfo> loadMaps( std::span<const char> str )
{
    ZoneScoped;
    enum AST {
        eName,
        eIdentifier,
        eIdentifierOrPop,
        eValue,
        eAssignValue,
        eAssignPush,
        ePush,
    };
    AST expectedTokens = eName;

    enum AssignTo {
        eNone,
        eEnemies,
        eTop,
        eBottom,
        eLeft,
        eRight,
        eFront,
        eBack,
        ePreview,
    };
    AssignTo assignTo = eNone;

    MapCreateInfo mod;
    std::pmr::vector<MapCreateInfo> levels;
    levels.reserve( 5 );

    using std::literals::string_view_literals::operator""sv;
    auto whitespace = U" \t\n\r"sv;

    cfg::TokenIterator it{ cfg::c_separators, str };
    for ( auto token = *it; ( token = *it ); ++it ) {
        if ( whitespace.find( static_cast<char32_t>( token.userEnum ) ) != std::u32string_view::npos ) { continue; }
        switch ( expectedTokens ) {
        case eName:
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            mod.name = std::u32string{ token.data, token.data + token.length }; // TODO utf-32
            expectedTokens = eAssignPush;
            continue;

        case eAssignPush:
            assert( token.userEnum == '=' );
            expectedTokens = ePush;
            continue;

        case ePush:
            assert( token.userEnum == '{' );
            expectedTokens = eIdentifier;
            continue;

        case eIdentifierOrPop:
            if ( token.userEnum == '}' ) {
                levels.push_back( mod );
                mod = {};
                expectedTokens = eName;
                continue;
            }
            [[fallthrough]];
        case eIdentifier: {
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            std::string_view v = *token;
            expectedTokens = eAssignValue;
            if ( v == "enemies"sv ) { assignTo = eEnemies; continue; }
            if ( v == "top"sv ) { assignTo = eTop; continue; }
            if ( v == "bottom"sv ) { assignTo = eBottom; continue; }
            if ( v == "left"sv ) { assignTo = eLeft; continue; }
            if ( v == "right"sv ) { assignTo = eRight; continue; }
            if ( v == "front"sv ) { assignTo = eFront; continue; }
            if ( v == "back"sv ) { assignTo = eBack; continue; }
            if ( v == "preview"sv ) { assignTo = ePreview; continue; }
            assert( !"unhandled identifier" );
            continue;
        }

        case eAssignValue:
            assert( token.userEnum == '=' );
            expectedTokens = eValue;
            continue;

        case eValue:
            assert( token.userEnum == cfg::TokenIterator::c_unknown );
            switch ( assignTo ) {
            case eEnemies: mod.enemies = static_cast<uint32_t>( std::atoi( token.data ) ); break;
            case eTop: mod.filePath[ MapCreateInfo::eTop ] = *token; break;
            case eBottom: mod.filePath[ MapCreateInfo::eBottom] = *token; break;
            case eLeft: mod.filePath[ MapCreateInfo::eLeft ] = *token; break;
            case eRight: mod.filePath[ MapCreateInfo::eRight ] = *token; break;
            case eFront: mod.filePath[ MapCreateInfo::eFront ] = *token; break;
            case eBack: mod.filePath[ MapCreateInfo::eBack ] = *token; break;
            case ePreview: mod.previewPath = *token; break;
            default: assert( !"unhandled variable" );
            }
            assignTo = eNone;
            expectedTokens = eIdentifierOrPop;
            continue;
        }

    }
    assert( expectedTokens == eName );
    assert( assignTo == eNone );

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

void Game::onExit()
{
    for ( auto& it : m_jetsContainer ) {
        delete it.model;
    }
}

void Game::onResize( uint32_t w, uint32_t h )
{
    ZoneScoped;
    m_screenTitle.resize( { w, h } );
    m_screenCustomize.resize( { w, h } );
    m_screenSettings.resize( { w, h } );
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

    g_uiProperty.m_colorA = color::dodgerBlue;
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



    m_screenPause = ScreenPause{
        &m_uiRings
        , [this](){ changeScreen( Screen::eGame ); }
        , [this](){ changeScreen( Screen::eGame, m_click ); }
        , [this](){ changeScreen( Screen::eMissionSelection, m_click ); }
    };

    m_screenTitle = ScreenTitle{
        &m_uiRings
        , [this](){ changeScreen( Screen::eMissionSelection, m_click ); }
        , [this](){ changeScreen( Screen::eCustomize, m_click ); }
        , [this](){ changeScreen( Screen::eSettings, m_click ); }
        , [this](){ quit(); }
    };

    m_screenSettings = ScreenSettings{
        &m_uiRings
        , [this](){ changeScreen( Screen::eMainMenu, m_click ); }
    };

    m_screenWin = ScreenWinLoose{
          color::winScreen
        , &m_uiRings
        , ui::loc::missionWin
        , [this](){ changeScreen( Screen::eMissionSelection, m_click ); }
    };
    m_screenLoose = ScreenWinLoose{
          color::crimson
        , &m_uiRings
        , ui::loc::missionLost
        , [this](){ changeScreen( Screen::eMissionSelection, m_click ); }
    };

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
    m_screenMissionSelect = ScreenMissionSelect{
        std::span<const MapCreateInfo>{ m_mapsContainer.begin(), m_mapsContainer.end() }
        , &m_uiRings
        , [this](){ if ( m_screenMissionSelect.prev() ) { m_audio->play( m_click ); } }
        , [this](){ if ( m_screenMissionSelect.next() ) { m_audio->play( m_click ); } }
        , [this](){ changeScreen( Screen::eMainMenu, m_click ); }
        , [this](){ changeScreen( Screen::eGameBriefing, m_click ); }
    };

    {
        std::pmr::vector<uint8_t> jetscfg = m_io->getWait( "jets.cfg" );
        const char* ptr = reinterpret_cast<const char*>( jetscfg.data() );
        std::span<const char> str{ ptr, ptr + jetscfg.size() };
        m_jetsContainer = loadJets( str );
        assert( !m_jetsContainer.empty() );
        for ( auto& it : m_jetsContainer ) {
            it.model = new Model( it.model_file, m_textures[ it.model_texture ], m_renderer, it.scale );
        }
    }

    std::pmr::vector<CustomizeData> data{};
    data.reserve( m_jetsContainer.size() );
    std::transform( m_jetsContainer.begin(), m_jetsContainer.end(), std::back_inserter( data ),
        []( ModelProto& it ) { return CustomizeData{ it.name, it.model, 1.0f / it.scale }; }
    );

    m_screenCustomize = ScreenCustomize{
        { 1, 2, 1 }
        , std::move( data )
        , &m_uiRings
        , [this](){ changeScreen( Screen::eMainMenu, m_click ); }
        , [this](){ if ( m_screenCustomize.prevJet() ) { m_audio->play( m_click ); } }
        , [this](){ if ( m_screenCustomize.nextJet() ) { m_audio->play( m_click ); } }
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
    rctx.projection = math::ortho( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ), -100.0f, 100.0f );
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

    case Screen::eDead:
        renderGameScreen( rctx );
        m_screenLoose.render( rctx );
        break;

    case Screen::eWin:
        renderGameScreen( rctx );
        m_screenWin.render( rctx );
        break;

    case Screen::eMissionSelection:
        m_screenMissionSelect.render( rctx );
        break;

    case Screen::eMainMenu:
        renderBackground( rctx );
        {
            auto rctx2 = rctx;
            rctx2.projection = math::perspective( 55.0_deg, 1280.0f / 720.0f, 0.001f, 2000.0f );
            m_spaceDust.render( rctx2 );
        }
        m_screenTitle.render( rctx );
        break;

    case Screen::eSettings:
        renderBackground( rctx );
        m_screenSettings.render( rctx );
        break;

    case Screen::eCustomize:
        renderBackground( rctx );
        m_screenCustomize.render( rctx );
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
        m_screenLoose.update( updateContext );
        break;
    case Screen::eWin:
        m_screenWin.update( updateContext );
        break;
    case Screen::eMainMenu:
        m_spaceDust.update( updateContext );
        m_screenTitle.update( updateContext );
        break;
    case Screen::eCustomize:
        m_screenCustomize.update( updateContext );
        break;
    case Screen::eSettings:
        m_screenSettings.update( updateContext );
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
    m_spaceDust.setCenter( m_jet.position() );
    m_spaceDust.setVelocity( m_jet.velocity() * -0.5f );
    m_spaceDust.update( updateContext );

    for ( Explosion& it : m_explosions ) {
        it.update( updateContext );
    }
    m_explosions.erase( std::remove_if( m_explosions.begin(), m_explosions.end(), &Explosion::isInvalid ), m_explosions.end() );

    {
        for ( Bullet*& b : m_bullets ) {
            assert( b );
            b->update( updateContext );
            for ( Enemy* e : m_enemies ) {
                assert( e );
                if ( !intersectLineSphere( b->position(), b->prevPosition(), e->position(), 15.0_m ) ) {
                    continue;
                }
                e->setDamage( b->damage() );
                b->kill();
                m_hudData.score += b->score();
                break;
            }

            if ( b->status() == SAObject::Status::eDead ) {
                std::destroy_at( b );
                m_poolBullets.dealloc( b );
                b = nullptr;
            }

        }

        m_bullets.erase( std::remove( m_bullets.begin(), m_bullets.end(), nullptr ), m_bullets.end() );
    }

    {
        const math::vec3 jetPos = m_jet.position();
        for ( Bullet*& b : m_enemyBullets ) {
            b->update( updateContext );
            if ( intersectLineSphere( b->position(), b->prevPosition(), jetPos, 15.0_m ) ) {
                m_jet.setDamage( b->damage() );
                b->kill();
            }
            if ( b->status() == SAObject::Status::eDead ) {
                std::destroy_at( b );
                m_poolBullets.dealloc( b );
                b = nullptr;
            }
        }
        m_enemyBullets.erase( std::remove( m_enemyBullets.begin(), m_enemyBullets.end(), nullptr ), m_enemyBullets.end() );
    }
    {
        for ( auto& e : m_enemies ) {
            e->update( updateContext );
            if ( e->status() == Enemy::Status::eDead ) {
                m_jet.untarget( e );
                m_explosions.push_back( Explosion{ e->position(), e->velocity(), m_plasma, 0.0f } );
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
    Bullet* bullet = m_jet.weapon( wID, m_poolBullets.alloc() );
    m_bullets.push_back( bullet );
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
    m_jet.setTarget( m_enemies[ random() % m_enemies.size() ] );
}

void Game::clearMapData()
{
    ZoneScoped;
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
    auto weap = m_screenCustomize.weapons();
    m_jet.setWeapon( m_weapons[ weap[ 0 ] ], 0 );
    m_jet.setWeapon( m_weapons[ weap[ 1 ] ], 1 );
    m_jet.setWeapon( m_weapons[ weap[ 2 ] ], 2 );

    assert( m_enemies.empty() );
    m_enemies.resize( mapInfo.enemies );
    for ( Enemy*& it : m_enemies ) {
        it = new ( m_poolEnemies.alloc() ) Enemy( m_enemyModel );
        it->setTarget( &m_jet );
        it->setWeapon( m_weapons[ 3 ] );
    }

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
        m_spaceDust.setVelocity(  math::vec3{ 1.0f, -0.1f, -0.4f }  );
        m_spaceDust.setCenter( {} );
        m_spaceDust.setLineWidth( 2.0f );
        m_currentScreen = scr;
        break;

    case Screen::eGame:
    case Screen::eGamePaused:
        m_spaceDust.setVelocity( m_jet.velocity() * -0.5f );
        m_spaceDust.setCenter( m_jet.position() );
        m_spaceDust.setLineWidth( 1.618f );
        m_currentScreen = scr;
        break;

    case Screen::eDead:
    case Screen::eWin:
        m_screenLoose.setScore( m_hudData.score );
        m_screenWin.setScore( m_hudData.score );
        [[fallthrough]];

    case Screen::eSettings:
    case Screen::eCustomize:
        m_currentScreen = scr;
        break;

    case Screen::eGameBriefing: {
        const uint32_t currentJet = m_screenCustomize.jet();
        createMapData( m_mapsContainer.at( m_screenMissionSelect.selectedMission() ), m_jetsContainer.at( currentJet ) );
        changeScreen( Screen::eGamePaused );
    } break;

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
        m_screenLoose.onMouseEvent( mouseEvent );
        break;

    case Screen::eWin:
        m_screenWin.onMouseEvent( mouseEvent );
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

void Game::renderGameScreen( RenderContext rctx )
{
    render3D( rctx );
    renderHUD( rctx );
}

void Game::renderHUD( RenderContext rctx )
{
    m_hud.render( rctx );
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
    m_spaceDust.render( rctx );
    for ( const Enemy* it : m_enemies ) {
        assert( it );
        it->render( rctx );
    }

    for ( const Bullet* it : m_bullets ) {
        assert( it );
        it->render( rctx );
    }
    for ( const Bullet* it : m_enemyBullets ) {
        assert( it );
        it->render( rctx );
    }
    m_jet.render( rctx );
}

void Game::renderBackground( RenderContext rctx ) const
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
        .m_color = color::dodgerBlue,
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

