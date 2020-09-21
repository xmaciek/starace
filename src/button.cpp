#include "button.hpp"

bool Button::isClicked( uint32_t x, uint32_t y ) const
{
    return m_enabled
        && ( x >= m_x )
        && ( x <= m_x + m_width )
        && ( y >= m_y )
        && ( y <= m_y + m_height );
}

void Button::draw()
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glTranslated( m_x, m_y, 0 );
    glBindTexture( GL_TEXTURE_2D, m_textureID );
    if ( m_enabled ) {
        glColor3f( 0, 0.75, 1 );
    }
    else {
        glColor3f( 0.3, 0.55, 0.65 );
    }
    glBegin( GL_QUADS );
    glTexCoord2d( 0, 0 );
    glVertex2d( 0, m_height );
    glTexCoord2d( 0, 1 );
    glVertex2d( 0, 0 );
    glTexCoord2d( 1, 1 );
    glVertex2d( m_width, 0 );
    glTexCoord2d( 1, 0 );
    glVertex2d( m_width, m_height );
    glEnd();

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
