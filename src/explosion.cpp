#include "explosion.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>

bool Explosion::isInvalid( const Explosion& e ) noexcept
{
    return e.m_state > 1.0f;
}

void Explosion::update( const UpdateContext& uctx )
{
    m_state += uctx.deltaTime;
    m_position += m_velocity * uctx.deltaTime;
}

void Explosion::render( const RenderContext& rctx ) const
{
    PushBuffer pushBuffer{};
    pushBuffer.m_verticeCount = 4;
    pushBuffer.m_pipeline = static_cast<PipelineSlot>( Pipeline::eSprite3D );
    pushBuffer.m_texture = m_texture;

    PushConstant<Pipeline::eSprite3D> pushConstant{};

    pushConstant.m_model = math::billboard( m_position, rctx.cameraPosition, rctx.cameraUp );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = math::lerp( color::yellowBlaster, color::crimson, m_state );
    pushConstant.m_color.a = 1.0f - m_state;
    const float size = std::lerp( 0.0f, 64.0_m, m_state );
    pushConstant.m_vertices[ 0 ] = { -size, -size, 0, 0 };
    pushConstant.m_vertices[ 1 ] = { -size, size, 0, 0 };
    pushConstant.m_vertices[ 2 ] = { size, size, 0, 0 };
    pushConstant.m_vertices[ 3 ] = { size, -size, 0, 0 };
    pushConstant.m_uv[ 0 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = { 0.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = { 1.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = { 1.0f, 0.0f, 0.0f, 0.0f };
    rctx.renderer->push( pushBuffer, &pushConstant );
}
