#pragma once

#include <ui/pipeline.hpp>

#include <extra/fnta.hpp>
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
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_lineHeight = 0;
    float m_scale = 1.0f;
    Texture m_texture{};
    using GlyphMap = FixedMapView<const char32_t, const fnta::Glyph>;
    GlyphMap m_glyphMap{};

public:
    struct CreateInfo {
        std::span<const uint8_t> fontAtlas{};
        Texture texture{};
        float scale = 1.0f;
    };

    ~Font() = default;
    Font( const CreateInfo& );

    float height() const;
    math::vec2 textGeometry( std::u32string_view ) const;

    using RenderText = std::pair<PushData, ui::PushConstant<ui::Pipeline::eSpriteSequence>>;
    RenderText composeText( const math::vec4& color, std::u32string_view ) const;
};

}
