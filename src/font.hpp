#pragma once

#include <shared/fixed_map.hpp>
#include <engine/render_context.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory_resource>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

class Font {
public:
    struct Glyph {
        glm::vec4 uv{};
        glm::vec2 size{};
        glm::vec2 advance{};
        glm::vec2 padding{};
    };

private:
    Texture m_texture{};
    uint32_t m_height = 0;
    FixedMap<char32_t, Glyph, 128> m_glyphs{};

public:
    ~Font();
    Font( const std::pmr::vector<uint8_t>& fontFileContent, uint32_t height );

    uint32_t height() const;
    uint32_t textLength( std::u32string_view );
    void renderText( RenderContext, const glm::vec4& color, double x, double y, std::u32string_view );
    using RenderText = std::pair<PushBuffer<Pipeline::eShortString>, PushConstant<Pipeline::eShortString>>;
    RenderText composeText( const glm::vec4& color, std::u32string_view );
};
