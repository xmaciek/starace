#pragma once

#include "render_context.hpp"
#include "sa.hpp"
#include "texture.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string_view>

class Font {
private:
    std::string m_name{};
    std::string m_stringTxt{};
    std::vector<uint32_t> m_charWidth{};
    std::vector<uint32_t> m_textures{};
    std::vector<glm::vec3> m_charData{};
    uint32_t m_height = 0;
    uint32_t m_listBase = 0;
    uint32_t m_middlePoint = 0;
    void makeDlist( TTF_Font* font, uint32_t ch );

public:
    ~Font();
    Font( const char* fontname, uint32_t h );

    uint32_t height() const;
    uint32_t middlePoint() const;
    uint32_t textLength( const char* text );
    uint32_t textLength( const std::string& text );
    void printText( double x, double y, const char* text );
    void renderText( RenderContext, const glm::vec4& color, double x, double y, std::string_view );
};
