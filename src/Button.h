#pragma once

#include "SA.h"
#include "Font.h"

class Button {
private:
    uint32_t m_x, m_y, m_width, m_height;
    uint32_t m_textureID;
    uint32_t m_bufferID;
    Font* m_font;
    float m_color[3];
    bool m_enabled;
    uint32_t m_text_length;

public:
    Button( Font* f = 0, uint32_t x = 0, uint32_t y = 0, uint32_t w = 192, uint32_t h = 48 );
    ~Button();
    std::string text;
    void SetTexture( uint32_t t );
    bool IsClicked( uint32_t x, uint32_t y );
    void MouseOver( uint32_t x, uint32_t y );
    void Draw();
    void UpdateCoord( uint32_t x, uint32_t y );
    void SetFont( Font *F);
    void SetSize( uint32_t w, uint32_t h );
    void Enable( bool b );
    void Enable();
    void Disable();
    bool IsEnabled();
    void SetText( const char *txt );
};
