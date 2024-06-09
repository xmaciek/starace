#include "speed_meter.hpp"

#include "circle.hpp"
#include "colors.hpp"
#include "game_pipeline.hpp"
#include "utils.hpp"
#include "units.hpp"
#include <ui/property.hpp>

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

static const auto c_circle = CircleGen<math::vec4>::getCircle<32>( 26.0f );
static const std::array<math::vec4, 5> c_fan{
    math::vec4{ -12.0f, 24.0f, 0.0f, 0.0f },
    math::vec4{ 0.0f, 26.5f, 0.0f, 0.0f },
    math::vec4{ 12.0f, 24.0f, 0.0f, 0.0f },
    math::vec4{ 3.0f, 0.0f, 0.0f, 0.0f },
    math::vec4{ -3.0f, 0.0f, 0.0f, 0.0f }
};

[[maybe_unused]]
static math::vec2 rightOf( const Label& w )
{
    return w.position() + math::vec2{ w.size().x, 0.0f };
};

SpeedMeter::SpeedMeter( std::nullptr_t ) noexcept
/*
: m_speed{ Label::CreateInfo{ .text = U"Speed: ", .font = g_uiProperty.fontSmall(), .position{ 38, 0 }, .color = color::winScreen, .anchor = Anchor::fBottom | Anchor::fLeft, } }
, m_speedValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .position = rightOf( m_speed ), .color = color::winScreen, .anchor = Anchor::fBottom | Anchor::fLeft, } }
*/
{
}

void SpeedMeter::render( ui::RenderContext ) const
{
    /*
    rctx.model = math::translate( rctx.model, math::vec3{ position(), 0.0f } );
    m_speed.render( rctx );
    m_speedValue.render( rctx );

    {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        std::copy( c_circle.begin(), c_circle.end(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), c_circle.size(), color::winScreen );

        PushBuffer pushBuffer{
            .m_pipeline = g_pipelines[ Pipeline::eLine3dStripColor ],
            .m_verticeCount = c_circle.size(),
            .m_lineWidth = 1.0f,
        };
        rctx.renderer->push( pushBuffer, &pushConstant );
    }

    {
        PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
        pushConstant.m_model = math::rotate( rctx.model, m_speedFanAngle, axis::z );
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        std::copy( c_fan.begin(), c_fan.end(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), 5, color::winScreen );

        PushBuffer pushBuffer{
            .m_pipeline = g_pipelines[ Pipeline::eTriangleFan3dColor ],
            .m_verticeCount = c_fan.size(),
        };
        rctx.renderer->push( pushBuffer, &pushConstant );
        pushConstant.m_model = math::rotate( pushConstant.m_model, 180.0_deg, axis::z );
        rctx.renderer->push( pushBuffer, &pushConstant );
    }
    */
}

void SpeedMeter::update( const UpdateContext& )
{
    /*
    m_speedFanAngle += math::radians( m_speedFan ) * uctx.deltaTime;
    */
}

void SpeedMeter::setSpeed( float )
{
    /*
    f = value2kmph( f );
    const uint32_t s = static_cast<uint32_t>( f );
    if ( s != static_cast<uint32_t>( m_speedFan ) ) {
        m_speedValue.setText( intToUTF32( s ) + U" km/h" );
    }
    m_speedFan = f;
    */
}
