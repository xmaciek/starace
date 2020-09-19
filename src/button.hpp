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
    Button( Font* F, GLuint X, GLuint Y, GLuint W, GLuint H );
    void SetTexture( GLuint t );
    bool IsClicked( GLuint X, GLuint Y ) const;
    void MouseOver( GLuint X, GLuint Y );
    void Draw();
    void UpdateCoord( GLuint X, GLuint Y );
    void SetFont( Font* F );
    void SetSize( const GLuint& W, const GLuint& H );
    void Enable( const bool& B );
    void Enable();
    void Disable();
    bool IsEnabled() const;
    void SetText( const char* txt );
};
