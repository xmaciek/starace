#pragma once

#include <ui/pipeline.hpp>

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/texture.hpp>
#include <shared/fixed_map.hpp>

#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

class Renderer;

namespace ui {

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

    using RenderText = std::pair<PushBuffer, ui::PushConstant<ui::Pipeline::eSpriteSequence>>;
    RenderText composeText( const math::vec4& color, std::u32string_view ) const;
};

}
