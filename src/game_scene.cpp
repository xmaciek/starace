#include "game_scene.hpp"

#include "colors.hpp"
#include "utils.hpp"

#include <profiler.hpp>

GameScene::GameScene( const std::array<Texture, 6>& t, Texture plasma ) noexcept
: m_skybox{ t }
, m_plasma{ plasma }
{
    ZoneScoped;
    m_explosions.reserve( 3000 );
    m_bullets.reserve( 200 );
    m_spacedust.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_spacedust.setCenter( {} );
    m_spacedust.setLineWidth( 2.0f );
}

void GameScene::render( const RenderContext& rctx )
{
    ZoneScoped;
    Enemy::renderAll( rctx, m_enemies );
    m_skybox.render( rctx );
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

void GameScene::update( const UpdateContext& uctx, math::vec3 jetPos, math::vec3 jetVel )
{
    ZoneScoped;
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

    auto extraExplosions = [this]( const Enemy& e ) -> bool
    {
        if ( e.status() != SAObject::Status::eDead ) return false;
        m_explosions.emplace_back( e.position(), e.velocity(), color::yellowBlaster, m_plasma, 64.0_m, 0.0f, 1.0f );
        return true;
    };
    std::erase_if( m_enemies, extraExplosions );
    std::ranges::for_each( m_enemies, [this]( Enemy& e ) { e.shoot( m_bullets ); } );

    m_spacedust.setCenter( jetPos );
    m_spacedust.setVelocity( -jetVel );
    m_spacedust.update( uctx );
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
