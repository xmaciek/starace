#ifndef SA_BUTTON
#define SA_BUTTON

#include "Font.h"
#include "SA.h"

class Button {
private:
    GLuint x, y, width, height, textureID;
    Font* font;
    GLfloat color[ 3 ];
    bool enabled;
    GLuint text_length;

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