#pragma once

#include "font.hpp"
#include "sa.hpp"

class Button {
private:
    GLuint x = 0;
    GLuint y = 0;
    GLuint width = 192;
    GLuint height = 48;
    GLuint textureID = 0;
    Font* font = nullptr;
    bool enabled = true;
    GLuint text_length = 0;

public:
    ~Button() = default;
    Button() = default;
    Button( Font* F, GLuint X, GLuint Y, GLuint W, GLuint H );
    std::string text;
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
