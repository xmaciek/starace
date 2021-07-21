#include "game.hpp"

#include "colors.hpp"
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
    const double sw = viewportWidth() / 2;
    const double sh = viewportHeight() / 2;
    rctx.model = glm::translate( rctx.model, glm::vec3{ sw, sh, 0.0f } );
    for ( size_t i = 0; i < 3; i++ ) {
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
        pushBuffer.m_texture = m_cyberRingTexture[ i ];

        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_model = glm::rotate( rctx.model, glm::radians( m_cyberRingRotation[ i ] ), glm::vec3{ 0.0f, 0.0f, 1.0f } );
        pushConstant.m_model = glm::translate( pushConstant.m_model, glm::vec3{ m_maxDimention * -0.5, m_maxDimention * -0.5, 0.0 } );
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_view = rctx.view;

        pushConstant.m_color = glm::vec4{ m_cyberRingColor[ i ][ 0 ], m_cyberRingColor[ i ][ 1 ], m_cyberRingColor[ i ][ 2 ], m_cyberRingColor[ i ][ 3 ] };

        pushConstant.m_vertices[ 0 ] = glm::vec4{ m_maxDimention, m_maxDimention, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ 0, m_maxDimention, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ m_maxDimention, 0, 0.0f, 0.0f };

        pushConstant.m_uv[ 0 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 1 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 2 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_uv[ 3 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };

        rctx.renderer->push( &pushBuffer, &pushConstant );
    }
}

void Game::renderCyberRingsMini( RenderContext rctx )
{
    const double sw = viewportWidth() / 2;
    const double sh = viewportHeight() / 2 - 8;
    rctx.model = glm::translate( rctx.model, glm::vec3{ sw, sh, 0.0 } );
    for ( size_t i = 0; i < 3; i++ ) {
        PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
        pushBuffer.m_texture = m_cyberRingTexture[ i ];
        PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
        pushConstant.m_color = glm::vec4{ m_hudColor4fv[ m_hudColor ][ 0 ], m_hudColor4fv[ m_hudColor ][ 1 ], m_hudColor4fv[ m_hudColor ][ 2 ], m_cyberRingColor[ i ][ 3 ] };
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_view = rctx.view;
        pushConstant.m_model = glm::rotate( rctx.model, glm::radians( m_cyberRingRotation[ i ] ), glm::vec3{ 0.0f, 0.0f, 1.0f } );

        pushConstant.m_vertices[ 0 ] = glm::vec4{ 32, 32, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ -32, 32, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ -32, -32, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ 32, -32, 0.0f, 0.0f };
        pushConstant.m_uv[ 0 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 1 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
        pushConstant.m_uv[ 2 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
        pushConstant.m_uv[ 3 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };

        rctx.renderer->push( &pushBuffer, &pushConstant );
    }
}

void Game::renderHUDBar( RenderContext rctx, const glm::vec4& xywh, float ratio )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ xywh.x, xywh.y, 0.0f } );

    {
        const glm::vec4 color{
            m_hudColor4fv[ m_hudColor ][ 0 ]
            , m_hudColor4fv[ m_hudColor ][ 1 ]
            , m_hudColor4fv[ m_hudColor ][ 2 ]
            , m_hudColor4fv[ m_hudColor ][ 3 ]
        };
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
        std::fill_n( pushConstant.m_colors.begin(), 4, color );
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

        pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
        pushConstant.m_vertices[ 1 ] = glm::vec4{ xywh.z, 0.0f, 0.0f, 0.0f };
        pushConstant.m_vertices[ 2 ] = glm::vec4{ xywh.z, ratio * xywh.w, 0.0f, 0.0f };
        pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0f, ratio * xywh.w, 0.0f, 0.0f };
        rctx.renderer->push( &pushBuffer, &pushConstant );
    }

}

void Game::renderPauseText( RenderContext rctx )
{
    renderCyberRings( rctx );
    renderHudTex( rctx, color::pause );

    m_btnQuitMission.render( rctx );
    constexpr static char PAUSED[] = "PAUSED";
    const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( PAUSED ) ) / 2;
    m_fontPauseTxt->renderText( rctx, color::yellow, posx, viewportHeight() - 128, PAUSED );
}

void Game::renderHudTex( RenderContext rctx, const glm::vec4& color )
{
    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
    pushBuffer.m_texture = m_hudTex;
    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = color;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ viewportWidth(), 0.0, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ viewportWidth(), viewportHeight(), 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0, viewportHeight(), 0.0f, 0.0f };
    pushConstant.m_uv[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void Game::renderHUD( RenderContext rctx )
{
    const glm::vec4 color{ m_hudColor4fv[ m_hudColor ][ 0 ]
        , m_hudColor4fv[ m_hudColor ][ 1 ]
        , m_hudColor4fv[ m_hudColor ][ 2 ]
        , m_hudColor4fv[ m_hudColor ][ 3 ]
    };

    char hudmessage[ 48 ]{};
    std::snprintf( hudmessage, sizeof( hudmessage ), "Shots: %d", m_shotsDone );
    m_fontGuiTxt->renderText( rctx, color, 320, viewportHeight() - 16, hudmessage );

    assert( m_jet );
    const uint32_t score = m_jet->score();;
    std::snprintf( hudmessage, sizeof( hudmessage ), "SCORE: %u", score );
    m_fontGuiTxt->renderText( rctx, color, 64, viewportHeight() - 100, hudmessage );
    /*radar*/
#if 0
    const glm::mat4x4 jetMat = glm::toMat4( m_jet->quat() );
    {
        PushConstant<Pipeline::eLine3dColor1> pushConstant{};
        pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ viewportWidth() - 80, 80, 0 } );;
        pushConstant.m_model *= jetMat;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        pushConstant.m_color = color;

        PushBuffer<Pipeline::eLine3dColor1> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_lineWidth = 1.0f;
        pushBuffer.m_vertices.reserve( 24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, -24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, -24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( 24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( 24, -24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, 24 );
        pushBuffer.m_vertices.emplace_back( -24, 24, -24 );
        pushBuffer.m_vertices.emplace_back( -24, -24, -24 );

        rctx.renderer->push( &pushBuffer, &pushConstant );
    }

    {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ viewportWidth() - 80, 80, 0 } );
        pushConstant.m_model *= jetMat;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;

        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{ rctx.renderer->allocator() };
        pushBuffer.m_lineWidth = 1.0f;
        pushBuffer.m_colors.resize( m_radar->segments() + 1, glm::vec4{ 0.3, 0.3f, 1.0f, 1.0f } );
        pushBuffer.m_vertices.reserve( m_radar->segments() + 1 );
        for ( size_t i = 0; i < m_radar->segments(); i++ ) {
            pushBuffer.m_vertices.emplace_back( m_radar->x( i ), m_radar->y( i ), 0 );
        }
        pushBuffer.m_vertices.emplace_back( m_radar->x( 0 ), m_radar->y( 0 ), 0 );
        rctx.renderer->push( &pushBuffer, &pushConstant );


        pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ viewportWidth() - 80, 80, 0 } );
        pushConstant.m_model *= jetMat;
        pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 90.0f ), glm::vec3{ 0, 1, 0 } );
        std::fill( pushBuffer.m_colors.begin(), pushBuffer.m_colors.end(), glm::vec4{ 0, 1, 0, 1 } );
        rctx.renderer->push( &pushBuffer, &pushConstant );

        pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ viewportWidth() - 80, 80, 0 } );
        pushConstant.m_model *= jetMat;
        pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 90.0f ), glm::vec3{ 1, 0, 0 } );
        std::fill( pushBuffer.m_colors.begin(), pushBuffer.m_colors.end(), glm::vec4{ 1, 0, 0, 1 } );
        rctx.renderer->push( &pushBuffer, &pushConstant );
    }
    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( const Enemy* e : m_enemies ) {
            e->drawRadarPosition( m_jet->position(), 92 );
        }
    }

    glDisable( GL_DEPTH_TEST );
    glm::vec3 cursor = m_jet->direction() * 92.0f;
    glLineWidth( 3 );
    glColor4f( 1, 1, 0, 0.9 );
    glBegin( GL_LINES );
    glVertex3d( 0, 0, 0 );

    glVertex3d( cursor.x, cursor.y, cursor.z );
    glEnd();
    glLineWidth( 1 );

#endif

    {
        RenderContext rctx2 = rctx;
        rctx2.model = glm::translate( rctx.model, glm::vec3{ 32, viewportHeight() / 2, 0  } );
        const std::string str = std::string{ "SPEED: " } + std::to_string( static_cast<int>( m_jet->speed() * 270 ) );
        m_fontGuiTxt->renderText( rctx2, color, 38, 0, str );
        {
            PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
            pushConstant.m_model = glm::rotate( rctx2.model, (float)glm::radians( m_speedAnim ), glm::vec3{ 0.0f, 0.0f, 1.0f } );
            pushConstant.m_view = rctx2.view;
            pushConstant.m_projection = rctx2.projection;
            std::fill_n( pushConstant.m_colors.begin(), 5, color );
            pushConstant.m_vertices[ 0 ] = { -3.0f, 0.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 1 ] = { 3.0f, 0.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 2 ] = { 12.0f, 24.0f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 3 ] = { 0.0f, 26.5f, 0.0f, 0.0f };
            pushConstant.m_vertices[ 4 ] = { -12.0f, 24.0f, 0.0f, 0.0f };

            PushBuffer<Pipeline::eTriangleFan3dColor> pushBuffer{};
            pushBuffer.m_verticeCount = 5;
            rctx2.renderer->push( &pushBuffer, &pushConstant );
            pushConstant.m_model = glm::rotate( pushConstant.m_model, glm::radians( 180.0f ), glm::vec3{ 0.0f, 0.0f, 1.0f } );
            rctx2.renderer->push( &pushBuffer, &pushConstant );
        }

        static std::array<glm::vec4, 32> ringVertices{};
        static std::array<glm::vec4, 32> ringColors{};
        static bool ringReady = false;
        if ( !ringReady ) {
            std::fill_n( ringColors.begin(), 32, color );
            for ( size_t i = 0; i < m_speedFanRing->segments(); ++i ) {
                ringVertices[ i ] = { m_speedFanRing->x( i ), m_speedFanRing->y( i ), 0.0f, 0.0f };
            }
            ringReady = true;
        }
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx2.model;
        pushConstant.m_view = rctx2.view;
        pushConstant.m_projection = rctx2.projection;
        pushConstant.m_vertices = ringVertices;
        pushConstant.m_colors = ringColors;
        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
        pushBuffer.m_lineWidth = 1.0f;
        pushBuffer.m_verticeCount = 32;
        rctx2.renderer->push( &pushBuffer, &pushConstant );
    }

    renderCyberRingsMini( rctx );

    assert( m_jet );
    const float power = (float)m_jet->energy();
    const float health = (float)m_jet->health();

    renderHUDBar( rctx, glm::vec4{ 12, 12, 36, 96 }, power / 100 );
    renderHUDBar( rctx, glm::vec4{ 64, 12, 36, 96 }, health / 100 );

    {
        std::string msg{ "FPS done: " };
        msg += std::to_string( m_fpsMeter.fps() );
        msg += ", calculated: ";
        msg += std::to_string( m_fpsMeter.calculated() );
        const double textMid = static_cast<double>( m_fontGuiTxt->textLength( "FPS done: XXX, calculated: xxxx.xx" ) ) / 2;
        const double posx = viewportWidth() / 2 - textMid;
        m_fontGuiTxt->renderText( rctx, color, posx, viewportHeight() - 28, msg );
    }
    m_fontPauseTxt->renderText( rctx, color, 10, 120, "PWR" );
    m_fontPauseTxt->renderText( rctx, color, 66, 120, "HP" );
}

void Game::render3D( RenderContext rctx )
{
    assert( m_jet );
    rctx.projection = glm::perspective( glm::radians( m_angle + m_jet->speed() * 6 ), static_cast<float>( viewportWidth() / viewportHeight() ), 0.001f, 2000.0f );
    rctx.view = glm::translate( rctx.view, glm::vec3{ 0, -0.255, -1 } );
    rctx.view *= glm::toMat4( m_jet->rotation() );
    rctx.view = glm::translate( rctx.view, -m_jet->position() );

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
    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = color::lightSkyBlue;
    pushConstant.m_uv[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
    pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ viewportWidth(), 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ viewportWidth(), viewportHeight(), 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0f, viewportHeight(), 0.0f, 0.0f };

    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
    pushBuffer.m_texture = m_menuBackground;

    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_menuBackgroundOverlay;
    pushConstant.m_color = glm::vec4{ 1, 1, 1, m_alphaValue };
    rctx.renderer->push( &pushBuffer, &pushConstant );

    pushBuffer.m_texture = m_starfieldTexture;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ m_maxDimention, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ m_maxDimention, m_maxDimention, 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0f, m_maxDimention, 0.0f, 0.0f };
    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void Game::renderWinScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderCyberRings( rctx );
    renderHudTex( rctx, color::winScreen );

    constexpr static char missionOK[] = "MISSION SUCCESSFUL";
    m_fontBig->renderText( rctx
        , color::white
        , viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( missionOK ) ) / 2
        , viewportHeight() - 128
        , missionOK
    );

    assert( m_jet );
    const uint32_t score = m_jet->score();;
    const std::string str = std::string{ "Your score: " } + std::to_string( score );
    m_fontBig->renderText( rctx
        , color::white
        , viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str.c_str() ) ) / 2
        , viewportHeight() - 128 - 36
        , str
    );
    m_btnReturnToMissionSelection.render( rctx );
}

void Game::renderDeadScreen( RenderContext rctx )
{
    renderGameScreen( rctx );
    renderCyberRings( rctx );
    renderHudTex( rctx, color::crimson );

    constexpr static char txt[] = "MISSION FAILED";
    m_fontBig->renderText( rctx
        , color::white
        , viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( txt ) ) / 2
        , viewportHeight() - 128
        , txt
    );

    assert( m_jet );
    const uint32_t score = m_jet->score();;
    const std::string str = std::string{ "Your score: " } + std::to_string( score );
    m_fontBig->renderText( rctx
        , color::white
        , viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str.c_str() ) ) / 2
        , viewportHeight() - 128 - 36
        , str
    );

    m_btnReturnToMissionSelection.render( rctx );
}

void Game::renderMissionSelectionScreen( RenderContext rctx )
{
    {
        MapProto& map = m_mapsContainer[ m_currentMap ];
        if ( map.preview_image.m_data == 0 ) {
            map.preview_image = loadTexture( map.preview_image_location.c_str() );
        }
    }

    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = color::white;
    pushConstant.m_uv[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = glm::vec4{ 1.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ m_maxDimention, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ m_maxDimention, m_maxDimention, 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ 0.0f, m_maxDimention, 0.0f, 0.0f };

    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
    pushBuffer.m_texture = m_mapsContainer[ m_currentMap ].preview_image;
    rctx.renderer->push( &pushBuffer, &pushConstant );
    {
        const std::string str = std::string{ "Map: " } + m_mapsContainer.at( m_currentMap ).name;
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->renderText( rctx, color::white, posx, viewportHeight() - 128, str );
    }
    {
        const std::string str = std::string{ "Enemies: " } + std::to_string( m_mapsContainer.at( m_currentMap ).enemies );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->renderText( rctx, color::white, posx, viewportHeight() - 148, str );
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
    m_fontPauseTxt->renderText( rctx, color::white, 192, viewportHeight() - 292, "Movement: AWSD QE" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, viewportHeight() - 310, "Speed controll: UO" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, viewportHeight() - 328, "Weapons: JKL" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, viewportHeight() - 346, "Targeting: I" );
    m_fontPauseTxt->renderText( rctx, color::white, 192, viewportHeight() - 380, "Press space to launch..." );
    renderHudTex( rctx, color::dodgerBlue );
    m_btnGO.render( rctx );
}

void Game::renderScreenCustomize( RenderContext rctx )
{
    renderClouds( rctx );
    renderCyberRings( rctx );
    renderHudTex( rctx, color::dodgerBlue );

    {
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( m_jetsContainer.at( m_currentJet ).name.c_str() ) ) / 2;
        m_fontBig->renderText( rctx,  color::white, posx, viewportHeight() - 64, m_jetsContainer.at( m_currentJet ).name );
    }
    {
        RenderContext rctx2 = rctx;
        rctx2.projection = glm::perspective(
            glm::radians( m_angle )
            , static_cast<float>( viewportWidth() / viewportHeight() )
            , 0.001f
            , 2000.0f
        );
        rctx2.model = glm::translate( rctx2.model, glm::vec3{ 0, -0.1, -1.25 } );
        rctx2.model = glm::rotate( rctx2.model, glm::radians( 15.0f ), glm::vec3{ 1, 0, 0 } );
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
