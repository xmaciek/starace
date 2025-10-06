#include "menu_scene.hpp"

#include "units.hpp"
#include "colors.hpp"
#include "game_pipeline.hpp"
#include <engine/math.hpp>

#include <profiler.hpp>

MenuScene::MenuScene( const CreateInfo& ci )
: m_background{ ci.background }
{
    m_spaceDust.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_spaceDust.setCenter( {} );
    m_spaceDust.setLineWidth( 2.0f );
}

void MenuScene::render( Renderer* renderer, math::vec2 viewport )
{
    ZoneScoped;

    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eBackground ],
        .m_verticeCount = 4,
    };
    pushBuffer.m_fragmentTexture[ 0 ] = m_background.texture;

    PushConstant<Pipeline::eBackground> pushConstant{
        .m_model = math::mat4( 1 ),
        .m_view = math::mat4( 1 ),
        .m_projection = math::ortho( 0.0f, viewport.x, 0.0f, viewport.y, -1.0f, 1.0f ),
        .m_color = color::dodgerBlue,
        .m_uv = m_background,
        .m_geometry = m_background.geometry(),
        .m_viewport = viewport,
    };
    renderer->push( pushBuffer, &pushConstant );

    const math::vec3 cameraPos = math::normalize( math::vec3{ -4, -3, -3 } ) * 24.0_m;
    RenderContext rctx{
        .renderer = renderer,
        .view = math::lookAt( cameraPos, math::vec3{}, math::vec3{ 0, 1, 0 } ),
        .projection = math::perspective( 55.0_deg, viewport.x / viewport.y, 0.001f, 2000.0f ),
        .viewport = viewport,
    };
    assert( m_model );
    m_model->render( rctx );
    m_spaceDust.render( rctx );
}

void MenuScene::update( const UpdateContext& uctx )
{
    m_spaceDust.update( uctx );
}
