#pragma once

#include <ui/pipeline.hpp>

#include <extra/fnta.hpp>
#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/texture.hpp>
#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>

#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

class Renderer;

namespace ui {

class Remapper;

class Font {
    const Font* m_upstream{};
    const Remapper* m_remapper{};
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_lineHeight = 0;
    float m_scale = 1.0f;
    Texture m_texture{};
    using Glyph = fnta::Glyph;
    using GlyphMap = FixedMapView<const char32_t, const Glyph>;
    GlyphMap m_glyphMap{};

    std::tuple<Glyph, Texture, math::vec2> getGlyph( char32_t ) const;

public:
    struct CreateInfo {
        std::span<const uint8_t> fontAtlas{};
        const Font* upstream = nullptr;
        const Remapper* remapper = nullptr;
        Texture texture{};
        float scale = 1.0f;
    };
    struct Sprite {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        operator math::vec4 () const noexcept;
        math::vec4 operator / ( const math::vec2& ) const noexcept;
    };

    ~Font() = default;
    Font() = default;
    Font( const CreateInfo& );
    Font( const Font& ) = delete;
    Font( Font&& );
    Font& operator = ( const Font& ) = delete;
    Font& operator = ( Font&& );

    Texture texture() const;
    float height() const;
    math::vec2 textGeometry( std::u32string_view ) const;
    math::vec2 extent() const;
    Sprite operator [] ( Hash::value_type ) const;

    using RenderText = std::pair<PushData, ui::PushConstant<ui::Pipeline::eSpriteSequence>>;
    RenderText composeText( const math::vec4& color, std::u32string_view ) const;
    void appendRenderText( math::vec2&, PushData&, ui::PushConstant<ui::Pipeline::eSpriteSequence>&, char32_t ) const;
};

}
