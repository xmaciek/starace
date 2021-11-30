#include "ui_image.hpp"

#include <renderer/renderer.hpp>
#include <renderer/pipeline.hpp>

#include <cassert>

UIImage::UIImage( Texture t )
: m_texture{ t }
{
    assert( t );
};

UIImage::UIImage( glm::vec2 position, glm::vec2 extent, glm::vec4 color, Texture t )
: Widget{ position, extent }
, m_color{ color }
, m_texture{ t }
{
    assert( t );
}

void UIImage::render( RenderContext rctx ) const
{
    assert( m_texture );
    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
    pushBuffer.m_texture = m_texture;

    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = m_color;

    const float x = m_position.x;
    const float y = m_position.y;
    const float xw = x + m_size.x;
    const float yh = y + m_size.y;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ x, yh, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ xw, yh, 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ xw, y, 0.0f, 0.0f };

    pushConstant.m_uv[ 0 ] = glm::vec4{ 0, 0, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec4{ 0, 1, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = glm::vec4{ 1, 1, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = glm::vec4{ 1, 0, 0.0f, 0.0f };

    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void UIImage::setColor( glm::vec4 c )
{
    m_color = c;
}
