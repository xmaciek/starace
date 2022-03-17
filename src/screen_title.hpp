#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "ui_glow.hpp"
#include "tab_order.hpp"

#include <engine/action.hpp>
#include <renderer/texture.hpp>

#include <cstdint>
#include <functional>
#include <string>

class Font;

class ScreenTitle : public Widget {
    TabOrder<> m_currentButton{};
    Widget* m_rings = nullptr;
    Glow m_glow{};
    Button m_newMission{};
    Button m_customize{};
    Button m_settings{};
    Button m_quit{};

    std::array<const Widget*, 4> widgets() const;
    std::array<Widget*, 4> widgets();

public:
    ~ScreenTitle() noexcept = default;
    ScreenTitle() noexcept = default;
    ScreenTitle(
          Widget* rings
        , std::function<void()>&& onMissionSelect
        , std::function<void()>&& onCustomize
        , std::function<void()>&& onSettings
        , std::function<void()>&& onQuit
    ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;
    void onAction( Action );

    void resize( math::vec2 );

};
