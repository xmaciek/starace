#pragma once

#include "button.hpp"
#include "game_action.hpp"
#include "label.hpp"
#include "map_create_info.hpp"
#include "tab_order.hpp"
#include "ui_glow.hpp"
#include "ui_image.hpp"
#include "widget.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <string_view>

class ScreenMissionSelect : public Widget
{
public:
    std::span<const MapCreateInfo> m_info;
    UIImage m_preview{};
    Glow m_glow{};
    Widget* m_rings = nullptr;
    Label m_title{};
    Label m_enemy{};
    Label m_enemyCount{};
    Button m_prev{};
    Button m_next{};
    Button m_cancel{};
    Button m_select{};

    TabOrder<> m_currentMission{};
    TabOrder<> m_currentWidget{ 2, 0, 4 };
    void updateTabOrderFocus( uint32_t, bool );
    void updatePreview();

public:
    ~ScreenMissionSelect() noexcept = default;
    ScreenMissionSelect() noexcept = default;

    ScreenMissionSelect(
        std::span<const MapCreateInfo>
        , Widget* rings
        , std::function<void()>&& onPrev
        , std::function<void()>&& onNext
        , std::function<void()>&& onCancel
        , std::function<void()>&& onSelect
    ) noexcept;

    uint32_t selectedMission() const;
    void resize( math::vec2 );
    bool next();
    bool prev();

    bool onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

};
