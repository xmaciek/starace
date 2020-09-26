#pragma once

#include "render_context.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class Font {
public:
    struct Glyph {
        uint32_t texture = 0;
        glm::vec2 size{};
        glm::vec2 advance{};
        glm::vec2 bearing{};
    };

private:
    std::string m_name{};
    std::vector<Glyph> m_glyphs{};
    uint32_t m_height = 0;

public:
    ~Font();
    Font( std::string_view, uint32_t h );

    uint32_t height() const;
    uint32_t textLength( std::string_view );
    void renderText( RenderContext, const glm::vec4& color, double x, double y, std::string_view );
};
