#pragma once

#include "colors.hpp"
#include "label.hpp"
#include "nineslice.hpp"

#include <renderer/texture.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>

class Font;
namespace ui {

class Button : public NineSlice {
private:
    Label m_label{};
    std::function<void()> m_onTrigger{};

public:
    ~Button() = default;
    Button() = default;
    Button( std::u32string_view, std::function<void()>&& onTrigger );
    Button( std::u32string_view, Anchor, std::function<void()>&& onTrigger );
    Button( std::function<void()>&& onTrigger );

    virtual bool onMouseEvent( const MouseEvent& ) override;
    virtual void render( RenderContext ) const override;
    virtual bool onAction( Action ) override;

    void setTrigger( std::function<void()> );
    void trigger() const;
    void setText( std::u32string_view );

};

}

using Button = ui::Button;
