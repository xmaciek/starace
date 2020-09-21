#include "button.hpp"

#include "renderer.hpp"
#include "render_pipeline.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    PushBuffer<Pipeline::eGuiTextureColor1> pushBuffer{ rctx.renderer->allocator() };
    pushBuffer.m_texture = m_textureID;

    PushConstant<Pipeline::eGuiTextureColor1> pushConstant;
    pushConstant.m_model = glm::translate( rctx.model, glm::vec3{ m_x, m_y, 0.0f } );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_vertices[ 0 ] = glm::vec2{ 0.0f, m_height };
    pushConstant.m_vertices[ 1 ] = glm::vec2{ 0.0f, 0.0f };
    pushConstant.m_vertices[ 2 ] = glm::vec2{ m_width, 0.0f };
    pushConstant.m_vertices[ 3 ] = glm::vec2{ m_width, m_height };
    pushConstant.m_uv[ 0 ] = glm::vec2{ 0.0f, 0.0f };
    pushConstant.m_uv[ 1 ] = glm::vec2{ 0.0f, 1.0f };
    pushConstant.m_uv[ 2 ] = glm::vec2{ 1.0f, 1.0f };
    pushConstant.m_uv[ 3 ] = glm::vec2{ 1.0f, 0.0f };
    pushConstant.m_color =  m_enabled
        ? glm::vec4{ 0.0f, 0.75f, 1.0f, 1.0f }
        : glm::vec4{ 0.3f, 0.55f, 0.65f, 1.0f };

    rctx.renderer->push( &pushBuffer, &pushConstant );

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glTranslated( m_x, m_y, 0 );

    if ( m_font ) {
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glTranslated( static_cast<float>( m_width ) / 2 - m_textLength, 0, 0 );
        m_font->printText( 0, static_cast<float>( m_height ) / 2 - m_font->middlePoint(), m_text.c_str() );
    }
    glPopMatrix();
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Button::updateCoord( uint32_t x, uint32_t y )
{
    m_x = x;
    m_y = y;
};

void Button::setTexture( uint32_t t )
{
    m_textureID = t;
}

Button::Button( Font* f, uint32_t x, uint32_t y, uint32_t w, uint32_t h )
: m_font{ f }
, m_x{ x }
, m_y{ y }
, m_width{ w }
, m_height{ h }
{
}

void Button::setSize( uint32_t w, const uint32_t h )
{
    m_width = w;
    m_height = h;
}

void Button::setFont( Font* f )
{
    m_font = f;
    if ( m_font ) {
        m_textLength = m_font->textLength( m_text.c_str() ) / 2;
    }
}

void Button::setEnabled( bool b )
{
    m_enabled = b;
}

bool Button::isEnabled() const
{
    return m_enabled;
}

void Button::setText( const char* txt )
{
    m_text = txt;
    if ( m_font ) {
        m_textLength = m_font->textLength( m_text.c_str() ) / 2;
    }
    else {
        m_textLength = 0;
    }
}
