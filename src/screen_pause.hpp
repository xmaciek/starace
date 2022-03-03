#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "label.hpp"
#include "ui_glow.hpp"
#include "game_action.hpp"

#include <renderer/texture.hpp>

#include <cstdint>
#include <functional>
#include <string>

class ScreenPause : public Widget
{
    Glow m_glow{};
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
          Widget* rings
        , std::u32string_view txtPause, std::function<void()>&& cbUnpause
        , std::u32string_view txtResume, std::function<void()>&&
        , std::u32string_view txtExit, std::function<void()>&&
    ) noexcept;

    void resize( math::vec2 );
    void onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;
};
