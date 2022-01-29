#pragma once

#include "font.hpp"
#include "widget.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>

#include <string_view>
#include <type_traits>

class Label : public Widget {
private:
    Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    math::vec4 m_color{};
    math::vec2 m_textExtent{};
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( std::u32string_view, Font*, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, Font*, Anchor, const math::vec4& color );
    Label( Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( Font*, Anchor, const math::vec4& color );

    virtual void render( RenderContext ) const override;
    void setText( std::u32string_view );
};
