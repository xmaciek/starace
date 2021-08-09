#pragma once

#include "render_context.hpp"
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory_resource>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class Font {
public:
    struct Glyph {
        glm::vec4 uv{};
        glm::vec2 size{};
        glm::vec2 advance{};
        glm::vec2 padding{};
        uint32_t dataPitch = 0;
        std::pmr::vector<uint8_t> data{};
    };

private:
    std::pmr::string m_name{};
    std::pmr::vector<Glyph> m_glyphs{};
    uint32_t m_height = 0;
    Texture m_texture{};

public:
    ~Font();
    Font( std::string_view, uint32_t h );

    uint32_t height() const;
    uint32_t textLength( std::string_view );
    void renderText( RenderContext, const glm::vec4& color, double x, double y, std::string_view );
    using RenderText = std::pair<PushBuffer<Pipeline::eShortString>, PushConstant<Pipeline::eShortString>>;
    RenderText composeText( const glm::vec4& color, std::string_view );
};
