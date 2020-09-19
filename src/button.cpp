#include "button.hpp"

bool Button::IsClicked( GLuint X, GLuint Y ) const
{
    if ( !enabled ) {
        return false;
    }
    return ( X >= x ) && ( X <= x + width ) && ( Y >= y ) && ( Y <= y + height );
}

void Button::Draw()
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glTranslated( x, y, 0 );
    glBindTexture( GL_TEXTURE_2D, textureID );
    if ( enabled ) {
        glColor3f( 0, 0.75, 1 );
    }
    else {
        glColor3f( 0.3, 0.55, 0.65 );
    }
    glBegin( GL_QUADS );
    glTexCoord2d( 0, 0 );
    glVertex2d( 0, height );
    glTexCoord2d( 0, 1 );
    glVertex2d( 0, 0 );
    glTexCoord2d( 1, 1 );
    glVertex2d( width, 0 );
    glTexCoord2d( 1, 0 );
    glVertex2d( width, height );
    glEnd();

    if ( font ) {
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glTranslated( static_cast<float>( width ) / 2 - text_length, 0, 0 );
        font->PrintTekst( 0, static_cast<float>( height ) / 2 - font->GetMiddlePoint(), text.c_str() );
    }
    glPopMatrix();
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Button::MouseOver( GLuint, GLuint ){};

void Button::UpdateCoord( GLuint X, GLuint Y )
{
    x = X;
    y = Y;
};

void Button::SetTexture( GLuint t )
{
    textureID = t;
}

Button::Button( Font* F, GLuint X, GLuint Y, GLuint W, GLuint H )
{
    x = X;
    y = Y;
    width = W;
    height = H;
    font = F;
}

void Button::SetSize( const GLuint& W, const GLuint& H )
{
    width = W;
    height = H;
}

void Button::SetFont( Font* F )
{
    font = F;
    if ( font ) {
        text_length = font->GetTextLength( text.c_str() ) / 2;
    }
}

void Button::Enable( const bool& B )
{
    enabled = B;
}
void Button::Enable()
{
    enabled = true;
}
void Button::Disable()
{
    enabled = false;
}

bool Button::IsEnabled() const
{
    return enabled;
}

void Button::SetText( const char* txt )
{
    text = txt;
    if ( font ) {
        text_length = font->GetTextLength( text.c_str() ) / 2;
    }
    else {
        text_length = 0;
    }
}
