#pragma once

#include "button.hpp"
#include "game_action.hpp"
#include "label.hpp"
#include "map_create_info.hpp"
#include "ui_glow.hpp"
#include "ui_image.hpp"
#include "widget.hpp"

#include <renderer/texture.hpp>

#include <cstdint>
#include <functional>
#include <memory_resource>
#include <span>
#include <string>
#include <vector>

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

    uint32_t m_currentMission = 0;
    uint32_t m_currentTab = 2;
    void updateTabOrderFocus( uint32_t, bool );
    void updatePreview();

public:
    ~ScreenMissionSelect() noexcept = default;
    ScreenMissionSelect() noexcept = default;

    ScreenMissionSelect(
        std::span<const MapCreateInfo>,
        Widget* rings,
        std::u32string_view enemyTxt,
        std::u32string_view txtPrev, std::function<void()>&&,
        std::u32string_view txtNext, std::function<void()>&&,
        std::u32string_view txtCancel, std::function<void()>&&,
        std::u32string_view txtSelect, std::function<void()>&&
    ) noexcept;

    uint32_t selectedMission() const;
    void resize( math::vec2 );
    bool next();
    bool prev();

    void onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

};
