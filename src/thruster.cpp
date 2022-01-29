#include "thruster.hpp"

#include "circle.hpp"
#include "game_pipeline.hpp"
#include "constants.hpp"

#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

static constexpr float rad = 0.32f * 0.04285f;
static const std::array<math::vec4, 32> s_outter = CircleGen<math::vec4>::getCircle<32>( rad );
static const std::array<math::vec4, 32> s_inner = CircleGen<math::vec4>::getCircle<32>( rad * 0.6f );

Thruster::Thruster( float length, float )
{
    setLength( length );
}

void Thruster::setLength( float newLength )
{
    m_lengthRange = math::vec2{ newLength * 0.95f, newLength };
}

void Thruster::setColorScheme( const ColorScheme& col )
{
    m_colorScheme = col;
}

void Thruster::update( const UpdateContext& updateContext )
{
    constexpr static float wiggleDuration = 0.125f;
    m_wiggle = std::fmod( m_wiggle + updateContext.deltaTime, wiggleDuration );
    const float f = std::cos( constants::pi * ( m_wiggle / wiggleDuration - 0.25f ) );
    const float diff = m_lengthRange.y - m_lengthRange.x;
    m_length = m_lengthRange.x + f * diff;
}

void Thruster::renderAt( RenderContext rctx, const math::vec3& pos ) const
{
    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dColor ),
        .m_verticeCount = 33,
    };

    PushConstant<Pipeline::eTriangleFan3dColor> pushConstant{};
    pushConstant.m_model = math::translate( rctx.model, pos );
    pushConstant.m_model = math::scale( pushConstant.m_model, math::vec3{ 1.0f, 1.0f, m_length } );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_vertices[ 0 ] = { 0.0f, 0.0f, 1.0f, 0.0f };
    std::copy( s_inner.begin(), s_inner.end(), pushConstant.m_vertices.begin() + 1 );
    pushConstant.m_colors[ 0 ] = m_colorScheme[ 1 ];
    std::fill_n( pushConstant.m_colors.begin() + 1, 32, m_colorScheme[ 0 ] );
    rctx.renderer->push( pushBuffer, &pushConstant );

    std::copy( s_outter.begin(), s_outter.end(), pushConstant.m_vertices.begin() + 1 );
    pushConstant.m_colors[ 0 ] = m_colorScheme[ 3 ];
    std::fill_n( pushConstant.m_colors.begin() + 1, 32, m_colorScheme[ 2 ] );
    rctx.renderer->push( pushBuffer, &pushConstant );
}
