#include "ui_rings.hpp"

#include "constants.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>
#include <engine/math.hpp>

UIRings::UIRings( std::array<Texture,3> t ) noexcept
: m_texture{ t }
{
}

void UIRings::render( ui::RenderContext rctx ) const
{
    static constexpr std::array color = {
        math::vec4{ 1.0f, 1.0f, 1.0f, 0.8f },
        math::vec4{ 1.0f, 1.0f, 1.0f, 0.7f },
        math::vec4{ 1.0f, 1.0f, 1.0f, 0.6f },
    };
    const math::vec2 center = m_size * 0.5f;
    const float mx = std::max( m_size.x, m_size.y );

    rctx.model = math::translate( rctx.model, math::vec3{ center, 0.0f } );
    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_vertices[ 0 ] = math::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = math::vec4{ 0.0f, mx, 0.0f, 1.0f };
    pushConstant.m_vertices[ 2 ] = math::vec4{ mx, mx, 1.0f, 1.0f };
    pushConstant.m_vertices[ 3 ] = math::vec4{ mx, 0.0f, 1.0f, 0.0f };

    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eGuiTextureColor1 ),
        .m_verticeCount = 4,
    };
    for ( size_t i = 0; i < 3; i++ ) {
        assert( m_texture[ i ] );
        pushBuffer.m_texture = m_texture[ i ];
        pushConstant.m_model = math::rotate( rctx.model, m_angle[ i ], axis::z );
        pushConstant.m_model = math::translate( pushConstant.m_model, math::vec3{ mx * -0.5, mx * -0.5, 0.0 } );
        pushConstant.m_color = color[ i ];
        rctx.renderer->push( pushBuffer, &pushConstant );
    }
}

void UIRings::update( const UpdateContext& uctx )
{
    static constexpr auto warp = []( float f )
    {
        if ( f > constants::pi ) { return f - constants::pi * 2.0f; }
        if ( f < -constants::pi ) { return f + constants::pi * 2.0f; }
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
