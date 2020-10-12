#pragma once

#include "label.hpp"
#include "render_context.hpp"
#include <renderer/texture.hpp>

#include <cstdint>
#include <string_view>

class Font;

class Button {
private:
    uint32_t m_x = 0;
    uint32_t m_y = 0;
    uint32_t m_width = 192;
    uint32_t m_height = 48;
    Label m_label{};
    Texture m_textureID{};
    bool m_enabled = true;

public:
    ~Button() = default;
    Button() = default;
    Button( std::string_view, Font*, Texture texture );
    Button( Font*, Texture texture );

    bool isClicked( uint32_t x, uint32_t y ) const;
    bool isEnabled() const;
    void render( RenderContext );
    void setEnabled( bool );
    void setPosition( uint32_t x, uint32_t y );
    void setSize( uint32_t w, uint32_t h );
    void setText( std::string_view );
    void setTexture( Texture t );
};
