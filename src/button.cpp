#include "button.hpp"

#include "colors.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>


Button::Button( std::string_view txt, Font* f, Texture texture )
: m_label( txt, Label::HAlign::eCenter, Label::VAlign::eMiddle, f, glm::vec2{ 0.5f * m_width, 0.5f * m_height }, color::white )
, m_textureID{ texture }
{
}

Button::Button( Font* f, Texture texture )
: m_label( f, Label::HAlign::eCenter, Label::VAlign::eMiddle, glm::vec2{ 0.5f * m_width, 0.5f * m_height }, color::white )
, m_textureID{ texture }
{
}

bool Button::isClicked( uint32_t x, uint32_t y ) const
{
    return m_enabled
        && ( x >= m_x )
        && ( x <= m_x + m_width )
        && ( y >= m_y )
        && ( y <= m_y + m_height );
}

void Button::render( RenderContext rctx )
{
    rctx.model = glm::translate( rctx.model, glm::vec3{ m_x, m_y, 0.0f } );
    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{};
    pushBuffer.m_texture = m_textureID;

    PushConstant<Pipeline::eGuiTextureColor1> pushConstant;
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_vertices[ 0 ] = glm::vec4{ 0.0f, m_height, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec4{ m_width, 0.0f, 0.0f, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec4{ m_width, m_height, 0.0f, 0.0f };
    pushConstant.m_uv[ 0 ] = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 2 ] = glm::vec4{ 1.0f, 1.0f, 0.0f, 0.0f };
    pushConstant.m_uv[ 3 ] = glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
    pushConstant.m_color =  m_enabled
        ? color::lightSkyBlue
        : color::lightSteelBlue;

    rctx.renderer->push( &pushBuffer, &pushConstant );

    m_label.render( rctx );
}

void Button::setPosition( uint32_t x, uint32_t y )
{
    m_x = x;
    m_y = y;
};

void Button::setTexture( Texture t )
{
    m_textureID = t;
}

void Button::setSize( uint32_t w, const uint32_t h )
{
    m_width = w;
    m_height = h;
}

void Button::setEnabled( bool b )
{
    m_enabled = b;
}

bool Button::isEnabled() const
{
    return m_enabled;
}

void Button::setText( std::string_view txt )
{
    m_label.setText( txt );
}
