#include "Button.h"

#include "shader.hpp"

bool Button::IsClicked( uint32_t x, uint32_t y ) {
    if ( !m_enabled ) {
        return false;
    }
    return ( x >= m_x ) && ( x <= m_x + m_width ) && ( y >= m_y ) && ( y <= m_y + m_height );
}


void Button::Draw() {
    if ( m_enabled ) {
        SHADER::setColor( 0, 0.75, 1, 1 );
    } else {
        SHADER::setColor( 0.3, 0.55, 0.65, 1 );
    }
    // TODO: reenable texture
//     glBindTexture(GL_TEXTURE_2D, textureID);

    SHADER::pushMatrix();
    SHADER::translate( m_x, m_y, 0 );
    if ( m_bufferID == 0 ) {
        SetSize( m_width, m_height );
    }
    SHADER::draw( GL_TRIANGLES, m_bufferID, 6 );

    if ( m_font ) {
      m_font->PrintTekst( m_width / 2 - m_text_length, m_height / 2 - m_font->GetMiddlePoint(), text );
    }
    SHADER::popMatrix();
}

void Button::MouseOver( uint32_t x, uint32_t y ) {};

void Button::UpdateCoord( uint32_t x, uint32_t y ) {
    m_x = x;
    m_y = y;
};

void Button::SetTexture( uint32_t t ) {
    m_textureID = t;
}

Button::Button( Font* f, uint32_t x, uint32_t y, uint32_t w, uint32_t h ) {
    m_x = x;
    m_y = y;
    m_width = w;
    m_height = h;
    m_font = f;
    m_enabled = true;
    m_textureID = 0;
    m_bufferID = 0;
    m_text_length = 0;
}

Button::~Button() {};

void Button::SetSize( uint32_t w, uint32_t h ) {
    m_width = w;
    m_height = h;
    SHADER::deleteBuffer( m_bufferID );
    m_bufferID = SHADER::getQuad( 0, 0, m_width, m_height );
}

void Button::SetFont( Font *F ) {
    m_font = F;
    if ( m_font ) {
        m_text_length = m_font->GetTextLength( text.c_str() ) / 2;
    }
}

void Button::Enable( bool b ) { m_enabled = b; }
void Button::Enable() { m_enabled = true; }
void Button::Disable() { m_enabled = false; }

bool Button::IsEnabled() { return m_enabled; }

void Button::SetText(const char* txt) {
    text = txt;
    if ( m_font ) {
        m_text_length = m_font->GetTextLength( text.c_str() ) / 2;
    } else {
        m_text_length = 0;
    }
}

