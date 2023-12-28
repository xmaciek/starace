#include "ui_rings.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>
#include <engine/math.hpp>

UIRings::UIRings( Texture t ) noexcept
: m_texture{ t }
{
}

void UIRings::render( ui::RenderContext rctx ) const
{
    static const std::array color = {
        math::vec4{ 1.0f, 1.0f, 1.0f, 0.8f },
        math::vec4{ 1.0f, 1.0f, 1.0f, 0.7f },
        math::vec4{ 0.0f, 0.0f, 0.0f, 0.6f },
    };

    const auto [ my, mx ] = std::minmax( m_size.x, m_size.y );

    PushConstant<Pipeline::eUiRings> pushConstant{
        .m_view = math::translate( rctx.view, math::vec3{ mx * 0.5f, my * 0.5f, 0.0f } ),
        .m_projection = rctx.projection,
        .m_xywh{ 0.0f, 0.0f, mx, mx },
    };

    for ( uint32_t i = 0; i < 3; ++i ) {
        pushConstant.m_color[ i ] = color[ i ];
        pushConstant.m_modelMatrix[ i ] = rctx.model;
        pushConstant.m_modelMatrix[ i ] = math::rotate( pushConstant.m_modelMatrix[ i ], m_angle[ i ], axis::z );
        pushConstant.m_modelMatrix[ i ] = math::translate( pushConstant.m_modelMatrix[ i ], math::vec3{ mx * -0.5f, mx * -0.5f, 0.0f } );
    }

    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eUiRings ],
        .m_verticeCount = 6,
        .m_instanceCount = 3,
    };
    pushData.m_resource[ 1 ].texture = m_texture;

    rctx.renderer->push( pushData, &pushConstant );
}

void UIRings::update( const UpdateContext& uctx )
{
    static constexpr auto warp = []( float f )
    {
        if ( f > math::pi ) { return f - math::pi * 2.0f; }
        if ( f < -math::pi ) { return f + math::pi * 2.0f; }
        return f;
    };
    m_angle[ 0 ] = warp( m_angle[ 0 ] + 50.0_deg * uctx.deltaTime );
    m_angle[ 1 ] = warp( m_angle[ 1 ] - 30.0_deg * uctx.deltaTime );

    static float speed = 70.0_deg;
    if ( m_angle[ 2 ] <= 0.0_deg ) {
        speed = 70.0_deg;
    } else if ( m_angle[ 2 ] > 90.0_deg ) {
        speed = -40.0_deg;
    }
    m_angle[ 2 ] += speed * uctx.deltaTime;
}
