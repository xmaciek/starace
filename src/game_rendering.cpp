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

void Game::renderGameScreenPaused( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderPauseText( rctx );
}

void Game::renderPauseText( RenderContext rctx )
{
    m_uiRings.render( rctx );
    renderHudTex( rctx, color::pause );

    m_btnQuitMission.render( rctx );
    m_lblPaused.setPosition( { viewportWidth() / 2, 128 } );
    m_lblPaused.render( rctx );
}

void Game::renderHudTex( RenderContext rctx, const glm::vec4& color )
{
    UIImage hudTex{ m_hudTex };
    hudTex.setColor( color );
    hudTex.setSize( { (float)viewportWidth(), (float)viewportHeight() } );
    hudTex.render( rctx );
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
        glm::perspective( glm::radians( m_angle + m_jet->speed() * 3 ), viewportAspect(), 0.001f, 2000.0f )
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
    m_uiRings.render( rctx );
    renderHudTex( rctx, color::dodgerBlue );
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

void Game::renderMissionSelectionScreen( RenderContext rctx )
{
    UIImage preview{ m_mapsContainer[ m_currentMap ].texture[ Map::Wall::ePreview ] };
    preview.setSize( { m_maxDimention, m_maxDimention } );
    preview.render( rctx );
    {
        // TODO: hud labels
        std::pmr::u32string str{ U"Map: " };
        const std::string& name = m_mapsContainer[ m_currentMap ].name;
        std::copy( name.begin(), name.end(), std::back_inserter( str ) );
        const double posx = (double)viewportWidth() / 2.0 - static_cast<double>( m_fontPauseTxt->textLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->renderText( rctx, color::white, posx, 128, str );
    }
    {
        // TODO: hud labels
        const std::pmr::u32string str = U"Enemies: " + intToUTF32( m_mapsContainer[ m_currentMap ].enemies );
        const double posx = (double)viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( str ) ) / 2;
        m_fontPauseTxt->renderText( rctx, color::white, posx, 148, str );
    }

    m_uiRings.render( rctx );
    renderHudTex( rctx, color::dodgerBlue );
    m_btnStartMission.render( rctx );
    m_btnReturnToMainMenu.render( rctx );
    m_btnNextMap.render( rctx );
    m_btnPrevMap.render( rctx );
}

void Game::renderGameScreenBriefing( RenderContext rctx )
{
    renderGameScreen( rctx );
    m_uiRings.render( rctx );

    // TODO: hud labels
    m_fontPauseTxt->renderText( rctx, color::white, 192, 292, U"Movement: AWSD QE" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, 310, U"Speed: UO" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, 328, U"Weapons: JKL" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, 346, U"Targeting: I" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, 380, U"Press space to launch..." );
    renderHudTex( rctx, color::dodgerBlue );
    m_btnGO.render( rctx );
}

void Game::renderScreenCustomize( RenderContext rctx )
{
    renderClouds( rctx );
    m_uiRings.render( rctx );
    renderHudTex( rctx, color::dodgerBlue );

    {
        // TODO: hud labels
        std::pmr::u32string jetName{};
        const std::string& name = m_jetsContainer[ m_currentJet ].name;
        std::copy( name.begin(), name.end(), std::back_inserter( jetName ) );
        const double posx = (double)viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( jetName ) ) / 2;
        m_fontBig->renderText( rctx,  color::white, posx, 64, jetName );
    }
    {
        RenderContext rctx2 = rctx;
        rctx2.projection = glm::perspective(
            glm::radians( m_angle )
            , viewportAspect()
            , 0.001f
            , 2000.0f
        );
        rctx2.model = glm::translate( rctx2.model, glm::vec3{ 0, 0.1, -1.25 } );
        rctx2.model = glm::rotate( rctx2.model, glm::radians( -15.0f ), glm::vec3{ 1, 0, 0 } );
        rctx2.model = glm::rotate( rctx2.model, glm::radians( (float)m_modelRotation ), glm::vec3{ 0, 1, 0 } );
        assert( m_previewModel );
        m_previewModel->render( rctx2 );
    }

    m_btnNextJet.render( rctx );
    m_btnPrevJet.render( rctx );
    m_btnCustomizeReturn.render( rctx );
    m_btnWeap1.render( rctx );
    m_btnWeap2.render( rctx );
    m_btnWeap3.render( rctx );
}
