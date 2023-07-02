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
        uint16_t position[ 2 ]{};
        uint16_t size[ 2 ]{};
        int16_t advance[ 2 ]{};
        int16_t padding[ 2 ]{};
    };

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_lineHeight = 0;
    Texture m_texture{};
    using GlyphMap = FixedMapView<const char32_t, const Glyph>;
    GlyphMap m_glyphMap{};

public:
    struct CreateInfo {
        std::span<const uint8_t> fontAtlas{};
        Texture texture{};
        uint32_t lineHeight = 0;
    };

    ~Font() = default;
    Font( const CreateInfo& );

    uint32_t height() const;
    float textLength( std::u32string_view ) const;

    using RenderText = std::pair<PushBuffer, ui::PushConstant<ui::Pipeline::eSpriteSequence>>;
    RenderText composeText( const math::vec4& color, std::u32string_view ) const;
};

}
