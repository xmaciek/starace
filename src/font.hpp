#pragma once

#include <shared/fixed_map.hpp>
#include <engine/render_context.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory_resource>
#include <cstdint>
#include <string_view>
#include <vector>
#include <utility>


class Renderer;
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

    struct CreateInfo {
        const std::pmr::vector<uint8_t>* fontFileContent = nullptr;
        Renderer* renderer = nullptr;
        std::u32string_view charset{};
    };
    Font( const CreateInfo&, uint32_t height );

    uint32_t height() const;
    float textLength( std::u32string_view ) const;
    void renderText( RenderContext, const glm::vec4& color, double x, double y, std::u32string_view ) const;
    using RenderText = std::pair<PushBuffer<Pipeline::eShortString>, PushConstant<Pipeline::eShortString>>;
    RenderText composeText( const glm::vec4& color, std::u32string_view ) const;
};
