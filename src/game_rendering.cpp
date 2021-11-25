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

void Game::renderHUDBar( RenderContext rctx, const glm::vec4& xywh, float ratio )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ xywh.x, (float)viewportHeight() - xywh.w - xywh.y, 0.0f } );
    {
        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
        pushBuffer.m_lineWidth = 2.0f;
        pushBuffer.m_verticeCount = 4;

        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_vertices[ 0 ] = { -4.0f, xywh.w + 4.0f, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = { xywh.z + 4.0f, xywh.w + 4.0f, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = { xywh.z + 4.0f, -4.0f, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = { -4.0f, -4.0f, 0.0f, 0.0f };
        std::fill_n( pushConstant.m_colors.begin(), 4, m_currentHudColor );
        rctx.renderer->push( &pushBuffer, &pushConstant );
    }

    {
        PushBuffer<Pipeline::eTriangleFan3dColor> pushBuffer{};
        pushBuffer.m_verticeCount = 4;
        PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        const glm::vec4 color{
            1.0f - ratio + colorHalf( ratio )
            , ratio + colorHalf( ratio )
            , 0.0f
            , 1.0f
        };
        std::fill_n( pushConstant.m_colors.begin(), 4, color );

        pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, xywh.w, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ xywh.z, xywh.w, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ xywh.z, xywh.w - ratio * xywh.w, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0f, xywh.w - ratio * xywh.w, 0.0f, 0.0f };
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
    const glm::vec4 color = m_currentHudColor;
    m_hud->render( rctx );
    {
        RenderContext rctx2 = rctx;
        rctx2.model = glm::translate( rctx.model, glm::vec3{ 32, viewportHeight() / 2, 0  } );

        // TODO: hud labels
        const std::pmr::u32string str = U"SPEED: " + intToUTF32( static_cast<int>( m_jet->speed() * 270 ) );
        m_fontGuiTxt->renderText( rctx2, color, 38, 0, str );

        {
            PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
            pushConstant.m_model = glm::rotate( rctx2.model, (float)glm::radians( m_speedAnim ), glm::vec3{ 0.0f, 0.0f, 1.0f } );
            pushConstant.m_view = rctx2.view;
            pushConstant.m_projection = rctx2.projection;
            std::fill_n( pushConstant.m_colors.begin(), 5, color );
            pushConstant.m_vertices[ 0 ] = { -12.0f, 24.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 1 ] = { 0.0f, 26.5f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 2 ] = { 12.0f, 24.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 3 ] = { 3.0f, 0.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 4 ] = { -3.0f, 0.0f, 0.0f, 0.0f };

            PushBuffer<Pipeline::eTriangleFan3dColor> pushBuffer{};
            pushBuffer.m_verticeCount = 5;
            rctx2.renderer->push( &pushBuffer, &pushConstant );
            pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 180.0f ), glm::vec3{ 0.0f, 0.0f, 1.0f } );
            rctx2.renderer->push( &pushBuffer, &pushConstant );
        }

        static const std::array ringVertices = CircleGen<glm::vec4>::getCircle<32>( 26.0f );

        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx2.model;
        pushConstant.m_view = rctx2.view;
        pushConstant.m_projection = rctx2.projection;
        std::copy_n( ringVertices.begin(), ringVertices.size(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), ringVertices.size(), color );

        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
        pushBuffer.m_lineWidth = 1.0f;
        pushBuffer.m_verticeCount = ringVertices.size();
        rctx2.renderer->push( &pushBuffer, &pushConstant );
    }

    assert( m_jet );
    const float power = (float)m_jet->energy();
    const float health = (float)m_jet->health();

    renderHUDBar( rctx, glm::vec4{ 12, 12, 36, 96 }, power / 100 );
    renderHUDBar( rctx, glm::vec4{ 64, 12, 36, 96 }, health / 100 );
    m_fontPauseTxt->renderText( rctx, color, 10, viewportHeight() - 120, U"PWR" );
    m_fontPauseTxt->renderText( rctx, color, 66, viewportHeight() - 120, U"HP" );

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
    renderHudTex( rctx, color::winScreen );

    // TODO: hud labels
    constexpr static char32_t missionOK[] = U"MISSION SUCCESSFUL";
    m_fontBig->renderText( rctx
        , color::white
        , (float)viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( missionOK ) ) / 2
        , 96
        , missionOK
    );

    assert( m_jet );
    const std::pmr::u32string str = U"Your score: " + intToUTF32( m_jet->score() );
    m_fontBig->renderText( rctx
        , color::white
        , (float)viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str ) ) / 2
        , 128
        , str
    );

    m_btnReturnToMissionSelection.render( rctx );
}

void Game::renderDeadScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderCyberRings( rctx );
    renderHudTex( rctx, color::crimson );

    // TODO: hud labels
    constexpr static char32_t txt[] = U"MISSION FAILED";
    m_fontBig->renderText( rctx
        , color::white
        , (float)viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( txt ) ) / 2
        , 96
        , txt
    );

    assert( m_jet );
    const std::pmr::u32string str = U"Your score: " + intToUTF32( m_jet->score() );
    m_fontBig->renderText( rctx
        , color::white
        , (float)viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str ) ) / 2
        , 128
        , str
    );

    m_btnReturnToMissionSelection.render( rctx );
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
