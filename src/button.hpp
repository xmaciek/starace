#pragma once

#include "font.hpp"
#include "label.hpp"
#include "render_context.hpp"

#include <cstdint>
#include <string_view>

class Button {
private:
    uint32_t m_x = 0;
    uint32_t m_y = 0;
    uint32_t m_width = 192;
    uint32_t m_height = 48;
    Label m_label{};
    uint32_t m_textureID = 0;
    bool m_enabled = true;

public:
    ~Button() = default;
    Button() = default;
    Button( std::string_view, Font*, uint32_t texture );
    Button( Font*, uint32_t texture );

    bool isClicked( uint32_t x, uint32_t y ) const;
    bool isEnabled() const;
    void render( RenderContext );
    void setEnabled( bool );
    void setPosition( uint32_t x, uint32_t y );
    void setSize( uint32_t w, uint32_t h );
    void setText( std::string_view );
    void setTexture( uint32_t t );
};
