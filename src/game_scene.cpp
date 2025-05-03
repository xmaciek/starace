#include "game_scene.hpp"

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
    m_skybox.render( rctx );
    Explosion::renderAll( rctx, m_explosions );
    Bullet::renderAll( rctx, m_bullets, m_plasma );
    m_spacedust.render( rctx );
}

void GameScene::update( const UpdateContext& uctx, math::vec3 jetPos, math::vec3 jetVel )
{
    ZoneScoped;
    Explosion::updateAll( uctx, m_explosions );

    Bullet::updateAll( uctx, m_bullets, m_explosions, m_plasma );
    std::erase_if( m_bullets, []( const Bullet& b ) { return b.m_type == Bullet::Type::eDead; } );

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

