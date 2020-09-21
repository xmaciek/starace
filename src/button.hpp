#pragma once

#include "font.hpp"
#include "sa.hpp"

class Button {
private:
    std::string m_text{};
    Font* m_font = nullptr;
    uint32_t m_x = 0;
    uint32_t m_y = 0;
    uint32_t m_width = 192;
    uint32_t m_height = 48;
    uint32_t m_textureID = 0;
    uint32_t m_textLength = 0;
    bool m_enabled = true;

public:
    ~Button() = default;
    Button() = default;
    Button( Font* f, uint32_t x, uint32_t y, uint32_t w, uint32_t h );

    bool isClicked( uint32_t x, uint32_t y ) const;
    bool isEnabled() const;
    void draw();
    void setEnabled( bool );
    void setFont( Font* f );
    void setSize( uint32_t w, uint32_t h );
    void setText( const char* txt );
    void setTexture( uint32_t t );
    void updateCoord( uint32_t x, uint32_t y );
};
