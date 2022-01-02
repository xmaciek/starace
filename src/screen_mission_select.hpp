#pragma once

#include "game_action.hpp"
#include "widget.hpp"
#include "label.hpp"
#include "button.hpp"

#include <renderer/texture.hpp>

#include <glm/vec4.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <memory_resource>

class Font;

struct MissionInfo {
    std::pmr::u32string m_title;
    Texture m_preview;
    uint32_t m_enemyCount = 0;
};

class ScreenMissionSelect : public Widget
{
public:
    std::pmr::vector<MissionInfo> m_info;
    UIImage m_preview{};
    UIImage m_glow{};
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
        std::pmr::vector<MissionInfo>&&,
        Font* fontSmall,
        Font* fontMedium,
        Font* fontLarge,
        Widget* rings,
        Texture glow,
        Texture button,
        std::u32string_view enemyTxt,
        std::u32string_view txtPrev, std::function<void()>&&,
        std::u32string_view txtNext, std::function<void()>&&,
        std::u32string_view txtCancel, std::function<void()>&&,
        std::u32string_view txtSelect, std::function<void()>&&
    ) noexcept;

    uint32_t selectedMission() const;
    void resize( glm::vec2 );
    bool next();
    bool prev();

    void onAction( Action );
    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

};
