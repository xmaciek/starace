#ifndef SA_BUTTON
#define SA_BUTTON

#include "font.hpp"
#include "sa.hpp"

class Button {
private:
    GLuint x = 0;
    GLuint y = 0;
    GLuint width = 0;
    GLuint height = 0;
    GLuint textureID = 0;
    Font* font = nullptr;
    bool enabled = false;
    GLuint text_length = 0;

public:
    Button();
    Button( Font* F, GLuint X, GLuint Y, GLuint W, GLuint H );
    ~Button();
    std::string text;
    void SetTexture( GLuint t );
    bool IsClicked( GLuint X, GLuint Y );
    void MouseOver( GLuint X, GLuint Y );
    void Draw();
    void UpdateCoord( GLuint X, GLuint Y );
    void SetFont( Font* F );
    void SetSize( const GLuint& W, const GLuint& H );
    void Enable( const bool& B );
    void Enable();
    void Disable();
    bool IsEnabled();
    void SetText( const char* txt );
};

#endif
