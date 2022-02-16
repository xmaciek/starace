#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "game_action.hpp"
#include "label.hpp"
#include "ui_glow.hpp"

#include <renderer/texture.hpp>

#include <functional>

class Font;

class ScreenWinLoose : public Widget
{
    Glow m_glow{};
    Widget* m_rings = nullptr;
    Label m_title{};
    Label m_score{};
    Label m_scoreValue{};
    Button m_continue{};

public:
    ~ScreenWinLoose() noexcept = default;
    ScreenWinLoose() noexcept = default;
    ScreenWinLoose(
        Texture button
        , math::vec4 color
        , Font* fontSmall
        , Font* fontLarge
        , Widget* rings
        , std::u32string_view title
        , std::function<void()>&& onContinue
    );

    bool onAction( Action );
    void setScore( uint32_t );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

    void resize( math::vec2 );

};
