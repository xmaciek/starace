#include "speed_meter.hpp"

#include "circle.hpp"
#include "colors.hpp"
#include "utils.hpp"

#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>


#include <glm/gtc/matrix_transform.hpp>

static const auto c_circle = CircleGen<glm::vec4>::getCircle<32>( 26.0f );
static const std::array<glm::vec4, 5> c_fan{
    glm::vec4{ -12.0f, 24.0f, 0.0f, 0.0f },
    glm::vec4{ 0.0f, 26.5f, 0.0f, 0.0f },
    glm::vec4{ 12.0f, 24.0f, 0.0f, 0.0f },
    glm::vec4{ 3.0f, 0.0f, 0.0f, 0.0f },
    glm::vec4{ -3.0f, 0.0f, 0.0f, 0.0f }
};

static glm::vec2 rightOf( const Label& w )
{
    return w.position() + glm::vec2{ w.size().x, 0.0f };
};

SpeedMeter::SpeedMeter( Font* font ) noexcept
: m_speed{ U"Speed: ", font, Anchor::fBottom | Anchor::fLeft, { 38, 0 }, color::winScreen }
, m_speedValue{ U"0", font, Anchor::fBottom | Anchor::fLeft, rightOf( m_speed ), color::winScreen }
{
}

void SpeedMeter::render( RenderContext rctx ) const
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ position(), 0.0f } );
    m_speed.render( rctx );
    m_speedValue.render( rctx );

    {
        PushConstant<Pipeline::eLine3dStripColor> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        std::copy( c_circle.begin(), c_circle.end(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), c_circle.size(), color::winScreen );

        PushBuffer<Pipeline::eLine3dStripColor> pushBuffer{};
        pushBuffer.m_lineWidth = 1.0f;
        pushBuffer.m_verticeCount = c_circle.size();
        rctx.renderer->push( &pushBuffer, &pushConstant );
    }

    {
        PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
        pushConstant.m_model = glm::rotate( rctx.model, m_speedFanAngle, axis::z );
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;
        std::copy( c_fan.begin(), c_fan.end(), pushConstant.m_vertices.begin() );
        std::fill_n( pushConstant.m_colors.begin(), 5, color::winScreen );
        PushBuffer<Pipeline::eTriangleFan3dColor> pushBuffer{};
        pushBuffer.m_verticeCount = c_fan.size();
        rctx.renderer->push( &pushBuffer, &pushConstant );
        pushConstant.m_model = glm::rotate( pushConstant.m_model, 180.0_deg, axis::z );
        rctx.renderer->push( &pushBuffer, &pushConstant );
    }
}

void SpeedMeter::update( const UpdateContext& uctx )
{
    m_speedFanAngle += glm::radians( m_speedFan ) * uctx.deltaTime;
}

void SpeedMeter::setSpeed( float f )
{
    f *= 270.0f;
    const uint32_t s = static_cast<uint32_t>( f );
    if ( s != static_cast<uint32_t>( m_speedFan ) ) {
        m_speedValue.setText( intToUTF32( s ) );
    }
    m_speedFan = f;
}
