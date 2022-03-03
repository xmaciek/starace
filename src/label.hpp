#pragma once

#include "font.hpp"
#include "widget.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>

#include <memory_resource>
#include <string_view>

class Font;

class Label : public Widget {
private:
    const Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    math::vec4 m_color{};
    math::vec2 m_textExtent{};
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( std::u32string_view, const Font*, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, const Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( std::u32string_view, const Font*, Anchor, const math::vec4& color );
    Label( const Font*, Anchor, const math::vec2& position, const math::vec4& color );
    Label( const Font*, Anchor, const math::vec4& color );

    virtual void render( RenderContext ) const override;
    void setText( std::u32string_view );
};
