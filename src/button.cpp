#include "button.hpp"

bool Button::IsClicked( GLuint X, GLuint Y ) const
{
    return m_enabled
        && ( X >= m_x )
        && ( X <= m_x + m_width )
        && ( Y >= m_y )
        && ( Y <= m_y + m_height );
}

void Button::Draw()
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
        m_font->PrintTekst( 0, static_cast<float>( m_height ) / 2 - m_font->GetMiddlePoint(), m_text.c_str() );
    }
    glPopMatrix();
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Button::MouseOver( GLuint, GLuint ){};

void Button::UpdateCoord( GLuint X, GLuint Y )
{
    m_x = X;
    m_y = Y;
};

void Button::SetTexture( GLuint t )
{
    m_textureID = t;
}

Button::Button( Font* F, GLuint X, GLuint Y, GLuint W, GLuint H )
: m_font{ F }
, m_x{ X }
, m_y{ Y }
, m_width{ W }
, m_height{ H }
{
}

void Button::SetSize( const GLuint& W, const GLuint& H )
{
    m_width = W;
    m_height = H;
}

void Button::SetFont( Font* F )
{
    m_font = F;
    if ( m_font ) {
        m_textLength = m_font->GetTextLength( m_text.c_str() ) / 2;
    }
}

void Button::Enable( const bool& B )
{
    m_enabled = B;
}
void Button::Enable()
{
    m_enabled = true;
}
void Button::Disable()
{
    m_enabled = false;
}

bool Button::IsEnabled() const
{
    return m_enabled;
}

void Button::SetText( const char* txt )
{
    m_text = txt;
    if ( m_font ) {
        m_textLength = m_font->GetTextLength( m_text.c_str() ) / 2;
    }
    else {
        m_textLength = 0;
    }
}
