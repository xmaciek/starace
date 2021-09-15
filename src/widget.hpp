#pragma once

#include "render_context.hpp"
#include "update_context.hpp"

#include <glm/vec2.hpp>

#include <array>

class Widget {
protected:
    glm::vec2 m_position{};
    glm::vec2 m_size{};

public:
    virtual ~Widget() noexcept = default;
    Widget() noexcept = default;
    inline Widget( glm::vec2 pos, glm::vec2 size ) noexcept
    : m_position{ pos }
    , m_size{ size }
    {}

    virtual void update( const UpdateContext& );
    virtual void render( RenderContext ) const = 0;

    void setPosition( glm::vec2 );
    void setSize( glm::vec2 );

    glm::vec2 size() const;
    glm::vec2 position() const;

};


class Layout : public Widget {
public:
    enum Flow {
        eHorizontal,
        eVertical,
    };

private:
    std::array<Widget*, 8> m_widgets{};
    using size_type = decltype( m_widgets )::size_type;
    size_type m_count = 0;
    Flow m_flow = Flow::eHorizontal;

public:

    ~Layout() noexcept = default;
    Layout() noexcept = default;
    inline explicit Layout( Flow a )
    : m_flow{ a }
    {}

    Layout( const Layout& ) = delete;
    Layout& operator = ( const Layout& ) = delete;

    // TODO: maybe do someting about moving
    Layout( Layout&& ) = delete;
    Layout& operator = ( Layout&& ) = delete;

    virtual void render( RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

    void add( Widget* );
};
