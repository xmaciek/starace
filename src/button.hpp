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
    std::function<void()> m_onTrigger{};
    bool m_focused = false;
    bool m_enabled = true;

    void updateColor();
public:
    ~Button() = default;
    Button() = default;
    Button( std::u32string_view, std::function<void()>&& onTrigger );
    Button( std::u32string_view, Anchor, std::function<void()>&& onTrigger );
    Button( std::function<void()>&& onTrigger );

    virtual bool onMouseEvent( const MouseEvent& ) override;
    virtual void render( RenderContext ) const override;

    void trigger() const;
    bool isEnabled() const;
    void setEnabled( bool );
    void setText( std::u32string_view );
    void setFocused( bool );
    bool isFocused() const;
};
