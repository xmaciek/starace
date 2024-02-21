#include "game_scene.hpp"

#include <Tracy.hpp>

GameScene::GameScene( const std::array<Texture, 6>& t ) noexcept
: m_skybox{ t }
{
    ZoneScoped;
    m_explosions.reserve( 3000 );
    m_explosions.clear();
    m_spacedust.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_spacedust.setCenter( {} );
    m_spacedust.setLineWidth( 2.0f );
}

void GameScene::render( const RenderContext& rctx )
{
    ZoneScoped;
    m_skybox.render( rctx );
    Explosion::renderAll( rctx, m_explosions );
    m_spacedust.render( rctx );
}

void GameScene::update( const UpdateContext& uctx, math::vec3 jetPos, math::vec3 jetVel )
{
    ZoneScoped;
    Explosion::updateAll( uctx, m_explosions );
    m_spacedust.setCenter( jetPos );
    m_spacedust.setVelocity( -jetVel );
    m_spacedust.update( uctx );
}

std::pmr::vector<Explosion>& GameScene::explosions()
{
    return m_explosions;
}