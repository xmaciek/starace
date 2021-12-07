#pragma once

#include "font.hpp"
#include "widget.hpp"

#include <engine/render_context.hpp>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <string_view>
#include <type_traits>

class Label : public Widget {
private:
    Font* m_font = nullptr;
    std::pmr::u32string m_text{};
    glm::vec4 m_color{};
    glm::vec2 m_textExtent{};
    bool m_isAutoSize = false;
    mutable Font::RenderText m_renderText{};

public:
    Label() = default;
    Label( std::u32string_view, Font*, const glm::vec2& position, const glm::vec4& color );
    Label( std::u32string_view, Font*, Anchor, const glm::vec2& position, const glm::vec4& color );
    Label( Font*, Anchor, const glm::vec2& position, const glm::vec4& color );

    virtual void render( RenderContext ) const override;
    void setText( std::u32string_view );
};
