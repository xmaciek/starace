#pragma once

#include "widget.hpp"

#include "button.hpp"
#include "game_action.hpp"
#include "label.hpp"
#include "ui_glow.hpp"

#include <functional>

class Font;

class ScreenSettings : public Widget
{
    Glow m_glow{};
    Widget* m_rings = nullptr;
    Label m_title{};
    Button m_btnReturn{};

public:
    ~ScreenSettings() noexcept = default;
    ScreenSettings() noexcept = default;
    ScreenSettings(
        Widget* rings
        , std::function<void()>&& onReturn
    ) noexcept;

    void onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

    void resize( math::vec2 );

};
