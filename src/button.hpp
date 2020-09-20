#pragma once

#include "font.hpp"
#include "sa.hpp"

class Button {
private:
    std::string m_text{};
    Font* m_font = nullptr;
    GLuint m_x = 0;
    GLuint m_y = 0;
    GLuint m_width = 192;
    GLuint m_height = 48;
    GLuint m_textureID = 0;
    GLuint m_textLength = 0;
    bool m_enabled = true;

public:
    ~Button() = default;
    Button() = default;
    Button( Font* f, GLuint x, GLuint y, GLuint w, GLuint h );

    bool isClicked( GLuint x, GLuint y ) const;
    bool isEnabled() const;
    void draw();
    void setEnabled( bool );
    void setFont( Font* f );
    void setSize( GLuint w, GLuint h );
    void setText( const char* txt );
    void setTexture( GLuint t );
    void updateCoord( GLuint x, GLuint y );
};
