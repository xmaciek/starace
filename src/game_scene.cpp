#include "game_scene.hpp"

#include "colors.hpp"
#include "utils.hpp"
#include "utils.hpp"

#include <profiler.hpp>

GameScene::GameScene( const CreateInfo& ci ) noexcept
: m_skybox{ ci.skybox }
, m_player{ ci.player }
, m_plasma{ ci.plasma }
, m_audio{ ci.audio }
{
    ZoneScoped;
    m_explosions.reserve( 3000 );
    m_bullets.reserve( 200 );
    m_spacedust.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_spacedust.setCenter( {} );
    m_spacedust.setLineWidth( 2.0f );

    m_enemies.resize( 20 );

    std::pmr::vector<uint16_t> callsigns( ci.enemyCallsignCount );
    std::iota( callsigns.begin(), callsigns.end(), 0 );
    std::shuffle( callsigns.begin(), callsigns.end(), Random{ std::random_device()() } );

    std::ranges::generate( m_enemies, [&ci, &callsigns, i=0u]() mutable
    {
        return Enemy::CreateInfo{
            .weapon = ci.enemyWeapon,
            .model = ci.enemyModel,
            .callsign = callsigns[ i++ ],
        };
    } );
}

void GameScene::render( const RenderContext& rctx )
{
    ZoneScoped;
    m_skybox.render( rctx );
    Enemy::renderAll( rctx, m_enemies );
    m_player.render( rctx );
    Explosion::renderAll( rctx, m_explosions );
    Bullet::renderAll( rctx, m_bullets, m_plasma );
    m_spacedust.render( rctx );
}

static void forEachQuadratic( auto& container1, auto& container2, auto&& fn )
{
    ZoneScoped;
    for ( auto&& i : container1 )
    for ( auto&& j : container2 )
        fn( i, j );
}

void GameScene::update( const UpdateContext& uctx )
{
    ZoneScoped;
    if ( m_pause ) [[unlikely]] return;

    m_look.setTarget( m_playerInput.lookAt ? 1.0f : 0.0f );
    m_look.update( uctx.deltaTime );
    m_player.setInput( m_playerInput );
    m_player.update( uctx );
    math::vec3 jetPos = m_player.position();
    math::vec3 jetVel = m_player.velocity();

    std::ranges::for_each( m_enemies, [s=m_player.signal()]( Enemy& e ) { e.setTarget( s ); } );
    Enemy::updateAll( uctx, m_enemies );
    Explosion::updateAll( uctx, m_explosions );

    Bullet::updateAll( uctx, m_bullets, m_explosions, m_plasma );
    std::erase_if( m_bullets, []( const Bullet& b ) { return b.m_type == Bullet::Type::eDead; } );

    uint32_t score = 0;

    auto makeExplosion = [plasma = m_plasma]( const Bullet& b, const math::vec3& p, float duration ) -> Explosion
    {
        return Explosion{
            .m_position = p + ( b.m_position - p ) * 15.0_m,
            .m_velocity = b.m_direction * b.m_speed * 0.1f,
            .m_color = color::white,
            .m_texture = plasma,
            .m_size = 16.0_m,
            .m_duration = duration
        };
    };
    auto testCollide = [&score, this, makeExplosion]( Enemy& e, Bullet& b )
    {
        if ( b.m_collideId == Enemy::COLLIDE_ID ) return;
        if ( !intersectLineSphere( b.m_position, b.m_prevPosition, e.position(), 15.0_m ) ) return;
        e.setDamage( std::exchange( b.m_damage, 0 ) ); // can hit multiple times
        score += b.m_score;
        b.m_type = Bullet::Type::eDead;
        m_explosions.emplace_back( makeExplosion( b, e.position(), 0.5f ) );
    };
    forEachQuadratic( m_enemies, m_bullets, testCollide );

    auto testCollide2 = [this, makeExplosion, jetPos]( Bullet& b )
    {
        if ( b.m_collideId == Player::COLLIDE_ID ) return;
        if ( !intersectLineSphere( b.m_position, b.m_prevPosition, jetPos, 15.0_m ) ) return;
        m_player.setDamage( b.m_damage );
        b.m_type = Bullet::Type::eDead;
        m_explosions.emplace_back( makeExplosion( b, jetPos, 0.5f ) );
    };
    std::ranges::for_each( m_bullets, testCollide2 );


    auto extraExplosions = [this]( const Enemy& e ) -> bool
    {
        if ( e.status() != SAObject::Status::eDead ) return false;
        m_explosions.emplace_back( e.position(), e.velocity(), color::yellowBlaster, m_plasma, 64.0_m, 0.0f, 1.0f );
        return true;
    };
    std::erase_if( m_enemies, extraExplosions );
    std::ranges::for_each( m_enemies, [this]( Enemy& e ) { e.shoot( m_bullets ); } );


    auto soundsToPlay = m_player.shoot( m_bullets );
    for ( auto&& s : soundsToPlay ) { if ( s ) m_audio->play( s, Audio::Channel::eSFX ); }

    m_spacedust.setCenter( jetPos );
    m_spacedust.setVelocity( -jetVel );
    m_spacedust.update( uctx );
    m_score += score;
}

void GameScene::onAction( input::Action a )
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
    case GameAction::eJetTarget:
        if ( a.digital() && a.value ) { retarget(); }
        break;
    default: break;
    }
}

void GameScene::retarget()
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
    auto proc = [f=std::numeric_limits<float>::max(), jetPos, jetDir, &s]( const Enemy& e ) mutable
    {
        float angle = math::angle( math::normalize( e.position() - jetPos ), jetDir );
        if ( angle > f ) return;
        f = angle;
        s = e.signal();
    };
    std::ranges::for_each( m_enemies, std::move( proc ) );
    m_player.setTarget( s );
}

std::tuple<math::vec3, math::vec3, math::vec3> GameScene::getCamera() const
{
    math::vec3 jetPos = m_player.position();
    math::vec3 jetCamPos = jetPos + m_player.cameraPosition() * m_player.rotation();
    math::vec3 jetCamUp = math::vec3{ 0, 1, 0 } * m_player.rotation();
    math::vec3 jetCamTgt = jetCamPos + m_player.cameraDirection();

    Signal tgt = m_player.target();
    math::vec3 lookAtTgt = tgt ? tgt.position : jetCamTgt;
    math::vec3 lookAtPos = math::vec3{ 0.0f, -20.0_m, 0.0f } * m_player.rotation() + jetPos - math::normalize( lookAtTgt - jetPos ) * 42.8_m;

    math::vec3 retPos = math::lerp( jetCamPos, lookAtPos, m_look.value() );
    math::vec3 retTgt = math::lerp( jetCamTgt, lookAtTgt, m_look.value() );

    return { retPos, jetCamUp, retTgt };
}

std::tuple<math::mat4, math::mat4> GameScene::getCameraMatrix( float aspect ) const
{
    const auto [ cameraPos, cameraUp, cameraTgt ] = getCamera();
    return {
        math::lookAt( cameraPos, cameraTgt, cameraUp ),
        math::perspective( math::radians( 55.0f + m_player.speed() * 3 ), aspect, 0.001f, 2000.0f )
    };
}

std::pmr::vector<Signal> GameScene::signals() const
{
    std::pmr::vector<Signal> r( m_enemies.size() );
    std::ranges::transform( m_enemies, r.begin(), []( const Enemy& p ) { return p.signal(); } );
    return r;
}

std::pmr::vector<Bullet>& GameScene::projectiles()
{
    return m_bullets;
}

std::pmr::vector<Explosion>& GameScene::explosions()
{
    return m_explosions;
}

std::pmr::vector<Enemy>& GameScene::enemies()
{
    return m_enemies;
}

Player& GameScene::player()
{
    return m_player;
}

const Player& GameScene::player() const
{
    return m_player;
}

bool GameScene::isPause() const
{
    return m_pause;
}

void GameScene::setPause( bool b )
{
    m_pause = b;
}

uint32_t GameScene::score() const
{
    return m_score;
}
