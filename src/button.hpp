#pragma once

#include "colors.hpp"
#include "label.hpp"
#include "ui_image.hpp"

#include <engine/render_context.hpp>
#include <renderer/texture.hpp>

#include <array>
#include <cstdint>
#include <string_view>

class Font;

class Button : public UIImage {
private:
    Label m_label{};
    bool m_mouseHover = false;
    bool m_enabled = true;

    void updateColor();
public:
    ~Button() = default;
    Button() = default;
    Button( std::u32string_view, Font*, Texture texture );
    Button( Font*, Texture texture );

    virtual bool onMouseEvent( const MouseEvent& ) override;
    virtual void render( RenderContext ) const override;

    bool isClicked( uint32_t x, uint32_t y ) const;
    bool isEnabled() const;
    void setEnabled( bool );
    void setText( std::u32string_view );
    void setTexture( Texture t );
};
