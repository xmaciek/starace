#pragma once

#include "tab_order.hpp"

#include <config/config.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <engine/mouse_event.hpp>
#include <engine/action.hpp>

#include <vector>
#include <memory>
#include <memory_resource>

class Widget;

namespace ui {

class Screen {
    TabOrder<> m_tabOrder{};
    math::vec2 m_extent{ 1, 1 };
    math::vec2 m_resize{ 1, 1 };
    math::vec2 m_viewport{ 1, 1 };
    math::vec2 m_offset{};
    std::pmr::vector<std::unique_ptr<Widget>> m_widgets{};

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

    void onAction( Action );
    void onMouseEvent( const MouseEvent& );
    void update( const UpdateContext& );
    void render( const RenderContext& ) const;
    void resize( math::vec2 );
};





}
