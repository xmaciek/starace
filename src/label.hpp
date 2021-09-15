#pragma once

#include "font.hpp"
#include "render_context.hpp"
#include "widget.hpp"

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <string_view>

class Label : public Widget {
public:
    enum struct HAlign {
        eLeft,
        eCenter,
    };
    enum struct VAlign {
        eBottom,
        eMiddle,
        eTop,
    };

private:
    Font* m_font = nullptr;
    mutable Font::RenderText m_renderText{};
    std::pmr::string m_text{};
    glm::vec4 m_color{};
    glm::vec2 m_positionOffset{};
    HAlign m_halign = HAlign::eLeft;
    VAlign m_valign = VAlign::eBottom;

    template <typename T>
    static inline std::string toString( T t ) { return std::to_string( t ); }
    static inline std::string toString( const char* t ) { return t; }

public:
    Label() = default;
    Label( Font*, HAlign, VAlign, const glm::vec2&, const glm::vec4& );
    Label( std::string_view, HAlign, VAlign, Font*, const glm::vec2&, const glm::vec4& );
    Label( std::string_view, Font*, const glm::vec2&, const glm::vec4& );

    virtual void render( RenderContext ) const override;
    void setText( std::string_view );

    template <typename T>
    void arg( const T& t )
    {
        setText( toString( t ) );
    }
};
