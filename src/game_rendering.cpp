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
    [[maybe_unused]]
    const auto [ w, h, a ] = viewport();
    const float u = w / 8;
    const float v = h / 8;

    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eBackground ),
        .m_verticeCount = 4,
        .m_texture = m_bg,
    };

    PushConstant<Pipeline::eBackground> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = color::dodgerBlue;

    const float x = 0;
    const float y = 0;
    const float xw = x + w;
    const float yh = y + h;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ x, yh, 0.0f, v };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ xw, yh, u, v };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ xw, y, u, 0.0f };

    rctx.renderer->push( pushBuffer, &pushConstant );

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
