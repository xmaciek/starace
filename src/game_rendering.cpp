#include "game.hpp"

#include "colors.hpp"
#include "ui_image.hpp"
#include "utils.hpp"

#include <renderer/buffer.hpp>
#include <renderer/pipeline.hpp>
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

void Game::renderCyberRings( RenderContext rctx )
{
    static constexpr std::array color = {
        glm::vec4{ 1.0f, 1.0f, 1.0f, 0.8f },
        glm::vec4{ 1.0f, 1.0f, 1.0f, 0.7f },
        glm::vec4{ 1.0f, 1.0f, 1.0f, 0.6f },
    };
    const double sw = (double)viewportWidth() / 2.0;
    const double sh = (double)viewportHeight() / 2.0;
    rctx.model = glm::translate( rctx.model, glm::vec3{ sw, sh, 0.0f } );
    for ( size_t i = 0; i < 3; i++ ) {
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
        pushBuffer.m_texture = m_cyberRingTexture[ i ];

        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_model = glm::rotate( rctx.model, m_cyberRingRotation[ i ], glm::vec3{ 0.0f, 0.0f, 1.0f } );
        pushConstant.m_model = glm::translate( pushConstant.m_model, glm::vec3{ m_maxDimention * -0.5, m_maxDimention * -0.5, 0.0 } );
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_view = rctx.view;
        pushConstant.m_color = color[ i ];

        pushConstant.m_vertices[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ 0, m_maxDimention, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ m_maxDimention, m_maxDimention, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ m_maxDimention, 0, 0.0f, 0.0f };

        pushConstant.m_uv[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_uv[ 1 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 2 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 3 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };

        rctx.renderer->push( &pushBuffer, &pushConstant );
    }
}

void Game::renderPauseText( RenderContext rctx )
{
    renderCyberRings( rctx );
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
        glm::perspective( glm::radians( m_angle + m_jet->speed() * 6 ), viewportAspect(), 0.001f, 2000.0f )
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
    renderCyberRings( rctx );
    renderHudTex( rctx, color::dodgerBlue );
    m_btnSelectMission.render( rctx );
    m_btnExit.render( rctx );
    m_btnCustomize.render( rctx );
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
    renderCyberRings( rctx );
    m_screenWin.render( rctx );
}

void Game::renderDeadScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderCyberRings( rctx );
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

    renderCyberRings( rctx );
    renderHudTex( rctx, color::dodgerBlue );
    m_btnStartMission.render( rctx );
    m_btnReturnToMainMenu.render( rctx );
    m_btnNextMap.render( rctx );
    m_btnPrevMap.render( rctx );
}

void Game::renderGameScreenBriefing( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderCyberRings( rctx );

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
    renderCyberRings( rctx );
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
