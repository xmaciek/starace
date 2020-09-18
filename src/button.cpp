#include "button.hpp"

bool Button::IsClicked( GLuint X, GLuint Y )
{
    if ( !enabled ) {
        return false;
    }
    if ( ( X >= x ) && ( X <= x + width ) && ( Y >= y ) && ( Y <= y + height ) ) {
        return true;
    }
    else {
        return false;
    }
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

    if ( font != NULL ) {
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glTranslated( width / 2 - text_length, 0, 0 );
        font->PrintTekst( 0, height / 2 - font->GetMiddlePoint(), text.c_str() );
    }
    glPopMatrix();
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Button::MouseOver( GLuint X, GLuint Y ){};

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
    enabled = true;
    textureID = 0;
}

Button::Button()
{
    textureID = 0;
    x = y = 0;
    width = 192;
    height = 48;
    enabled = true;
    font = NULL;
    text_length = 0;
}

void Button::SetSize( const GLuint& W, const GLuint& H )
{
    width = W;
    height = H;
}

void Button::SetFont( Font* F )
{
    font = F;
    if ( font != NULL ) {
        text_length = font->GetTextLength( text.c_str() ) / 2;
    }
}
Button::~Button(){};

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

bool Button::IsEnabled()
{
    return enabled;
}

void Button::SetText( const char* txt )
{
    text = txt;
    if ( font != NULL ) {
        text_length = font->GetTextLength( text.c_str() ) / 2;
    }
    else {
        text_length = 0;
    }
}
