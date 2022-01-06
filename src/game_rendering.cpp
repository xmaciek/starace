#include "game.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "ui_image.hpp"
#include "utils.hpp"

#include <renderer/buffer.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

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


std::tuple<glm::mat4, glm::mat4> Game::getCameraMatrix() const
{
    if ( !m_jet ) {
        return {};
    }

    glm::mat4 view = glm::translate( glm::mat4( 1.0f ), glm::vec3{ 0, 0.255, -1 } );
    view *= glm::toMat4( m_jet->rotation() );
    view = glm::translate( view, -m_jet->position() );
    return {
        view,
        glm::perspective( glm::radians( 55.0f + m_jet->speed() * 3 ), viewportAspect(), 0.001f, 2000.0f )
    };
}

void Game::render3D( RenderContext rctx )
{
    std::tie( rctx.view, rctx.projection ) = getCameraMatrix();

    assert( m_map );
    m_map->render( rctx );
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
    assert( m_jet );
    m_jet->render( rctx );
}

void Game::renderMainMenu( RenderContext rctx )
{
    renderClouds( rctx );
    m_screenTitle.render( rctx );
}

void Game::renderClouds( RenderContext rctx ) const
{
    const glm::vec2 size = { (float)viewportWidth(), (float)viewportHeight() };
    UIImage cloud1{ m_menuBackground };
    cloud1.setSize( size );
    cloud1.setColor( color::lightSkyBlue );
    cloud1.render( rctx );

    UIImage cloud2{ m_menuBackgroundOverlay };
    cloud2.setSize( size );
    cloud2.setColor( { 1, 1, 1, m_alphaValue } );
    cloud2.render( rctx );

    UIImage cloud3{ m_starfieldTexture };
    cloud3.setSize( { m_maxDimention, m_maxDimention } );
    cloud3.setColor( { 1, 1, 1, m_alphaValue } );
    cloud3.render( rctx );
}

void Game::renderWinScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    m_uiRings.render( rctx );
    m_screenWin.render( rctx );
}

void Game::renderDeadScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    m_uiRings.render( rctx );
    m_screenLoose.render( rctx );
}

void Game::renderScreenCustomize( RenderContext rctx )
{
    renderClouds( rctx );
    m_screenCustomize.render( rctx );
}
