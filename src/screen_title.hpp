#pragma once

#include "widget.hpp"
#include "button.hpp"
#include "ui_glow.hpp"

#include <engine/action.hpp>
#include <shared/rotary_index.hpp>
#include <renderer/texture.hpp>

#include <cstdint>
#include <functional>
#include <string>

class Font;

class ScreenTitle : public Widget {
    RotaryIndex<uint16_t, 0, 3> m_currentButton{};
    Widget* m_rings = nullptr;
    Glow m_glow{};
    Button m_newMission{};
    Button m_customize{};
    Button m_quit{};

public:
    ~ScreenTitle() noexcept = default;
    ScreenTitle() noexcept = default;
    ScreenTitle(
        Font*
        , Texture btn
        , Widget* rings
        , std::u32string_view mission, std::function<void()>&&
        , std::u32string_view customize, std::function<void()>&&
        , std::u32string_view quit, std::function<void()>&&
    ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;
    void onAction( Action );

    void resize( math::vec2 );

};
