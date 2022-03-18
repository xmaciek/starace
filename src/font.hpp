#pragma once

#include "game_pipeline.hpp"

#include <shared/fixed_map.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <memory_resource>
#include <cstdint>
#include <string_view>
#include <vector>
#include <utility>
#include <span>


class Renderer;
class Font {
public:
    struct Glyph {
        math::vec4 uv{};
        math::vec2 size{};
        math::vec2 advance{};
        math::vec2 padding{};
    };

private:
    Texture m_texture{};
    uint32_t m_height = 0;
    FixedMap<char32_t, Glyph, 128> m_glyphs{};

public:
    ~Font();

    struct CreateInfo {
        std::span<const uint8_t> fontFileContent{};
        Renderer* renderer = nullptr;
        std::u32string_view charset{};
    };
    Font( const CreateInfo&, uint32_t height );

    uint32_t height() const;
    float textLength( std::u32string_view ) const;
    void renderText( RenderContext, const math::vec4& color, double x, double y, std::u32string_view ) const;
    using RenderText = std::pair<PushBuffer, PushConstant<Pipeline::eSpriteSequence>>;
    RenderText composeText( const math::vec4& color, std::u32string_view ) const;
};
