#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "label.hpp"
#include "ui_image.hpp"

#include <renderer/texture.hpp>

#include <functional>

class Font;

class ScreenWinLoose : public Widget
{
    UIImage m_glow{};
    Label m_title{};
    Label m_score{};
    Label m_scoreValue{};
    Button m_continue{};

public:
    ~ScreenWinLoose() noexcept = default;
    ScreenWinLoose() noexcept = default;
    ScreenWinLoose(
        Texture glow
        , Texture button
        , glm::vec4 color
        , Font* fontSmall
        , Font* fontLarge
        , std::u32string_view title
        , std::function<void()>&& onContinue
    );

    virtual void render( RenderContext ) const override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

    void resize( glm::vec2 );

};
