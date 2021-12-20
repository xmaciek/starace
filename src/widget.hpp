#pragma once

#include "anchor.hpp"

#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <engine/mouse_event.hpp>

#include <glm/vec2.hpp>

#include <array>

class Widget {
protected:
    glm::vec2 m_position{};
    glm::vec2 m_size{};
    Anchor m_anchor = Anchor::fTop | Anchor::fLeft;
    uint16_t m_tabOrder = 0xFFFF;

    bool testRect( glm::vec2 ) const;
    glm::vec2 offsetByAnchor() const;

public:
    virtual ~Widget() noexcept = default;
    Widget() noexcept = default;
    inline Widget( Anchor a ) noexcept
    : m_anchor{ a }
    {}

    inline Widget( glm::vec2 pos, glm::vec2 size ) noexcept
    : m_position{ pos }
    , m_size{ size }
    {}

    inline Widget( glm::vec2 pos, glm::vec2 size, Anchor a ) noexcept
    : m_position{ pos }
    , m_size{ size }
    , m_anchor{ a }
    {}

    virtual void update( const UpdateContext& );
    virtual void render( RenderContext ) const = 0;
    virtual bool onMouseEvent( const MouseEvent& );

    void setPosition( glm::vec2 );
    void setSize( glm::vec2 );

    glm::vec2 size() const;
    glm::vec2 position() const;

    uint16_t tabOrder() const;
    void setTabOrder( uint16_t );
};


class Layout {
public:
    enum Flow {
        eHorizontal,
        eVertical,
    };

private:
    glm::vec2 m_position{};
    Flow m_flow = Flow::eHorizontal;

public:
    ~Layout() noexcept = default;
    Layout() noexcept = default;
    inline Layout( glm::vec2 pos, Flow a ) noexcept
    : m_position{ pos }
    , m_flow{ a }
    {}

    void operator () ( Widget** begin, Widget** end ) const;
};
