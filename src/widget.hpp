#pragma once

#include "anchor.hpp"

#include <engine/action.hpp>
#include <engine/math.hpp>
#include <engine/mouse_event.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

#include <span>

class Widget {
public:
    static constexpr uint16_t c_invalidTabOrder = 0xFFFF;

protected:
    math::vec2 m_position{};
    math::vec2 m_size{};
    Anchor m_anchor = Anchor::fTop | Anchor::fLeft;
    uint16_t m_tabOrder = c_invalidTabOrder;
    bool m_enabled : 1 = true;
    bool m_focused : 1 = false;

    bool testRect( math::vec2 ) const;
    static bool testRect( math::vec2 p, math::vec2 pos, math::vec2 size );
    static bool testRect( math::vec2 p, const math::vec4& xywh );

    math::vec2 offsetByAnchor() const;

public:
    virtual ~Widget() noexcept = default;
    Widget() noexcept = default;
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
    virtual void render( RenderContext ) const = 0;
    virtual bool onMouseEvent( const MouseEvent& );

    virtual bool isEnabled() const;
    virtual void setEnabled( bool );
    virtual bool isFocused() const;
    virtual void setFocused( bool );

    void setPosition( math::vec2 );
    void setSize( math::vec2 );

    math::vec2 size() const;
    math::vec2 position() const;

    uint16_t tabOrder() const;
    void setTabOrder( uint16_t );

    void setAnchor( Anchor );

    virtual bool onAction( Action );
};


class Layout {
public:
    enum Flow {
        eHorizontal,
        eVertical,
    };

private:
    math::vec2 m_position{};
    Flow m_flow = Flow::eHorizontal;

public:
    ~Layout() noexcept = default;
    Layout() noexcept = default;
    inline Layout( math::vec2 pos, Flow a ) noexcept
    : m_position{ pos }
    , m_flow{ a }
    {}

    void operator () ( std::span<Widget*> ) const;
};
