#pragma once

#include "button.hpp"
#include "label.hpp"
#include "widget.hpp"
#include "ui_glow.hpp"
#include "model.hpp"

#include <engine/action.hpp>
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <shared/rotary_index.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <memory_resource>
#include <string>
#include <vector>

class Font;

struct CustomizeData
{
    std::u32string_view m_jetName{};
    Model* m_jetModel{};
    float m_jetUiScale = 1.0f;
};

class ScreenCustomize : public Widget
{
    std::pmr::vector<CustomizeData> m_jets{};
    uint32_t m_currentJet = 0;
    float m_currentRotation = 0.0f;
    Widget* m_rings = nullptr;
    RotaryIndex<uint32_t, 0, 3> m_weap1{};
    RotaryIndex<uint32_t, 0, 3> m_weap2{};
    RotaryIndex<uint32_t, 0, 3> m_weap3{};
    uint32_t m_currentTabOrder = 0;

    Glow m_glow{};
    Label m_jetName{};
    Button m_jetPrev{};
    Button m_jetNext{};
    Button m_done{};

    Button m_btnWeap1{};
    Button m_btnWeap2{};
    Button m_btnWeap3{};

    void updateInfo();
    void updateFocus();

public:
    ~ScreenCustomize() noexcept = default;
    ScreenCustomize() noexcept = default;
    ScreenCustomize(
        std::array<uint32_t, 3> equipment
        , std::pmr::vector<CustomizeData>&& jets
        , Font* fontSmall
        , Font* fontMedium
        , Texture btn
        , Widget* rings
        , std::u32string_view done, std::function<void()>&&
        , std::u32string_view pJet, std::function<void()>&&
        , std::u32string_view nJet, std::function<void()>&&
        , std::function<void()>&& w1
        , std::function<void()>&& w2
        , std::function<void()>&& w3
    ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual bool onMouseEvent( const MouseEvent& ) override;

    void resize( math::vec2 );
    void onAction( Action );

    bool nextJet();
    bool prevJet();
    void nextWeap( uint32_t );

    std::array<uint32_t, 3> weapons() const;
    uint32_t jet() const;
};
