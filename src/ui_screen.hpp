#pragma once

#include <config/config.hpp>
#include <engine/math.hpp>
#include <engine/mouse_event.hpp>
#include <engine/update_context.hpp>
#include <shared/pmr_pointer.hpp>
#include <ui/input.hpp>
#include <ui/tab_order.hpp>

#include <vector>
#include <memory>
#include <memory_resource>

namespace ui {

struct RenderContext;
class Widget;

class Screen {
    TabOrder<> m_tabOrder{};
    math::vec2 m_extent{ 1, 1 };
    math::vec2 m_resize{ 1, 1 };
    math::vec2 m_viewport{ 1, 1 };
    math::vec2 m_offset{};
    float m_anim = 0.0f;
    std::pmr::vector<UniquePointer<Widget>> m_widgets{};

    UniquePointer<Widget> m_comboBoxList{};

    void changeFocus( uint16_t from, uint16_t to );
    Widget* findWidgetByTabOrder( uint16_t );

public:
    ~Screen() noexcept = default;
    Screen() noexcept = default;
    Screen( const Screen& ) = delete;
    Screen( Screen&& ) noexcept = default;
    Screen( const cfg::Entry& ) noexcept;

    Screen& operator = ( Screen&& ) noexcept = default;
    Screen& operator = ( const Screen& ) = delete;

    void onAction( ui::Action );
    void onMouseEvent( const MouseEvent& );
    void update( const UpdateContext& );
    void render( const RenderContext& ) const;
    void resize( math::vec2 );

    void show( math::vec2 size );
};

}
