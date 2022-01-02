#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "label.hpp"
#include "ui_image.hpp"
#include "game_action.hpp"

#include <renderer/texture.hpp>

#include <cstdint>
#include <functional>
#include <string>

class Font;

class ScreenPause : public Widget
{
    UIImage m_glow{};
    Widget* m_rings = nullptr;
    Button m_resume{};
    Button m_exit{};
    Label m_pause{};
    std::function<void()> m_unpause{};
    uint32_t m_currentTab = 1;

public:
    ~ScreenPause() noexcept = default;
    ScreenPause() noexcept = default;
    ScreenPause(
        Font* fontSmall
        , Font* fontMedium
        , Widget* rings
        , Texture glow
        , Texture button
        , std::u32string_view txtPause, std::function<void()>&& cbUnpause
        , std::u32string_view txtResume, std::function<void()>&&
        , std::u32string_view txtExit, std::function<void()>&&
    ) noexcept;

    void resize( glm::vec2 );
    void onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;
};
