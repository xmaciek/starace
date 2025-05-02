#pragma once

#include <engine/math.hpp>
#include <engine/mouse_event.hpp>
#include <shared/pmr_pointer.hpp>
#include <shared/hash.hpp>
#include <ui/input.hpp>
#include <ui/tab_order.hpp>
#include <ui/message_box.hpp>

#include <vector>
#include <memory>
#include <memory_resource>
#include <span>

namespace ui {

struct RenderContext;
struct UpdateContext;
class Widget;

class Screen {
    TabOrder<> m_tabOrder{};
    math::vec2 m_extent{ 1, 1 };
    math::vec2 m_resize{ 1, 1 };
    math::vec2 m_viewport{ 1, 1 };
    math::vec2 m_offset{};
    float m_anim = 0.0f;
    Hash::value_type m_scene = 0;
    Hash::value_type m_name = 0;
    std::pmr::vector<UniquePointer<Widget>> m_widgets{};

    UniquePointer<Widget> m_glow{};
    UniquePointer<Widget> m_modalWidget{};
    UniquePointer<Widget> m_footer{};

    enum class RepeatDirection : uint32_t {
        none,
        eUp,
        eDown,
        eRight,
        eLeft,
    };
    using enum RepeatDirection;
    RepeatDirection m_repeatDirection = none;
    float m_inputRepeatDelay = 0.0f;

    void changeFocus( uint16_t from, uint16_t to );
    Widget* findWidgetByTabOrder( uint16_t );

    void updateInputRepeat( float );

    std::pmr::memory_resource* allocator();

public:
    ~Screen() noexcept = default;
    Screen() noexcept = default;
    Screen( const Screen& ) = delete;
    Screen( Screen&& ) noexcept = default;
    Screen( std::span<const uint8_t> ) noexcept;

    Screen& operator = ( Screen&& ) noexcept = default;
    Screen& operator = ( const Screen& ) = delete;

    void onAction( ui::Action );
    void onMouseEvent( const MouseEvent& );
    void update( const UpdateContext& );
    void render( const RenderContext& ) const;
    void resize( math::vec2 );

    inline Hash::value_type name() const { return m_name; }
    inline Hash::value_type scene() const { return m_scene; }
    void show( math::vec2 size );
    void refreshInput();
    void lockitChanged();

    [[nodiscard]]
    UniquePointer<MessageBox> messageBox( Hash::value_type );
    void addModalWidget( UniquePointer<Widget>&& );

};

}
