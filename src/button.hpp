#pragma once

#include "colors.hpp"
#include "label.hpp"
#include "ui_image.hpp"

#include <engine/render_context.hpp>
#include <renderer/texture.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>

class Font;

class Button : public UIImage {
private:
    Label m_label{};
    std::function<void()> m_onClick{};
    bool m_mouseHover = false;
    bool m_enabled = true;

    void updateColor();
public:
    ~Button() = default;
    Button() = default;
    Button( std::u32string_view, Font*, Texture texture, std::function<void()>&& onClick );
    Button( Font*, Texture texture, std::function<void()>&& onClick );

    virtual bool onMouseEvent( const MouseEvent& ) override;
    virtual void render( RenderContext ) const override;

    bool isEnabled() const;
    void setEnabled( bool );
    void setText( std::u32string_view );
    void setTexture( Texture t );
};
