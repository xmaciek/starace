#pragma once

#include <ui/anchor.hpp>
#include <ui/input.hpp>

#include <engine/math.hpp>
#include <engine/mouse_event.hpp>
#include <shared/pmr_pointer.hpp>

#include <span>
#include <list>
#include <memory_resource>

class Renderer;

namespace ui {

class Screen;

struct RenderContext {
    Renderer* renderer = nullptr;
    math::mat4 model{ 1.0f };
    math::mat4 view{ 1.0f };
    math::mat4 projection{ 1.0f };

    math::vec4 colorMain{};
    math::vec4 colorFocus{};
};

struct UpdateContext {
    float deltaTime{};
};

enum class EventProcessing : uint32_t {
    eStop,
    eContinue,
};

class Widget {
    friend Screen;
    EventProcessing onEvent( const MouseEvent& );
    void onUpdate( const UpdateContext& );

public:
    static constexpr uint16_t INVALID_TAB = 0xFFFF;

    template <typename T>
    inline T* emplace_child( const typename T::CreateInfo& ci )
    {
        auto alloc = std::pmr::get_default_resource();
        auto& w = m_children.emplace_back( UniquePointer<T>{ alloc, ci } );
        return reinterpret_cast<T*>( w.get() );
    }

    void onRender( RenderContext ) const;

protected:
    std::pmr::list<UniquePointer<Widget>> m_children{};
    math::vec2 m_position{};
    math::vec2 m_size{};
    Anchor m_anchor = Anchor::fTop | Anchor::fLeft;
    uint16_t m_tabOrder = INVALID_TAB;
    bool m_enabled : 1 = true;
    bool m_focused : 1 = false;

    bool testRect( math::vec2 ) const;

    static bool testRect( math::vec2 p, math::vec2 pos, math::vec2 size );
    static bool testRect( math::vec2 p, const math::vec4& xywh );

public:
    virtual ~Widget() noexcept = default;
    Widget() noexcept = default;
    Widget( const Widget& ) = delete;
    Widget( Widget&& ) = delete;
    Widget& operator = ( const Widget& ) = delete;
    Widget& operator = ( Widget&& ) = delete;

    inline Widget( Anchor a ) noexcept
    : m_anchor{ a }
    {}

    inline Widget( math::vec2 pos, math::vec2 size ) noexcept
    : m_position{ pos }
    , m_size{ size }
    {}

    inline Widget( math::vec2 pos, math::vec2 size, Anchor a ) noexcept
    : m_position{ pos }
    , m_size{ size }
    , m_anchor{ a }
    {}

    virtual void update( const UpdateContext& );
    virtual void render( const RenderContext& ) const;
    virtual void refreshInput();
    virtual void lockitChanged();
    virtual EventProcessing onMouseEvent( const MouseEvent& );
    virtual EventProcessing onAction( ui::Action );

    virtual bool isEnabled() const;
    virtual void setEnabled( bool );
    virtual bool isFocused() const;
    virtual void setFocused( bool );

    void setPosition( math::vec2 );
    void setSize( math::vec2 );

    math::vec2 size() const;
    math::vec2 position() const;
    math::vec2 offsetByAnchor() const;

    uint16_t tabOrder() const;
    void setTabOrder( uint16_t );

    void setAnchor( Anchor );

};

template <typename T>
struct TabOrdering : public std::false_type {};

}
