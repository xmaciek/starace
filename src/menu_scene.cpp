#include "menu_scene.hpp"

#include "units.hpp"
#include "colors.hpp"
#include "game_pipeline.hpp"

#include <profiler.hpp>

MenuScene::MenuScene( const CreateInfo& ci )
: m_uiAtlasTexture{ ci.uiAtlasTexture }
, m_uiAtlasExtent{ ci.uiAtlasExtent }
, m_uiSlice{ ci.uiSlice }
{
    m_spaceDust.setVelocity( math::vec3{ 0.0f, 0.0f, 26.0_m } );
    m_spaceDust.setCenter( {} );
    m_spaceDust.setLineWidth( 2.0f );
}

void MenuScene::render( RenderContext rctx ) const
{
    ZoneScoped;
    const float w = rctx.viewport.x;
    const float h = rctx.viewport.y;
    const float a = w / h;
    const math::vec2 uv = math::vec2{ w, h } / m_uiAtlasExtent;

    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eBackground ],
        .m_verticeCount = 4,
    };
    pushBuffer.m_fragmentTexture[ 1 ] = m_uiAtlasTexture;

    PushConstant<Pipeline::eBackground> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = color::dodgerBlue,
        .m_uvSlice = m_uiSlice,
        .m_xyuv{
            math::vec4{ 0, 0, 0, 0 },
            math::vec4{ 0, h, 0, uv.y },
            math::vec4{ w, h, uv.x, uv.y },
            math::vec4{ w, 0, uv.x, 0 }
        },
    };
    rctx.renderer->push( pushBuffer, &pushConstant );

    assert( m_model );
    rctx.projection = math::perspective(
        55.0_deg
        , a
        , 0.001f
        , 2000.0f
    );
    const math::vec3 cameraPos = math::normalize( math::vec3{ -4, -3, -3 } ) * 24.0_m;
    rctx.view = math::lookAt( cameraPos, math::vec3{}, math::vec3{ 0, 1, 0 } );

    m_model->render( rctx );
    m_spaceDust.render( rctx );
}

void MenuScene::update( const UpdateContext& uctx )
{
    m_spaceDust.update( uctx );
}
