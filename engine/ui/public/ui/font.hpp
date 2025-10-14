#pragma once

#include <ui/pipeline.hpp>
#include <ui/sprite.hpp>

#include <extra/fnta.hpp>
#include <engine/math.hpp>
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

class Property;
class FontMap;

class Font {
    friend Property;
    friend FontMap;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_lineHeight = 0;
    float m_scale = 1.0f;
    Hash::value_type m_name{};
    Texture m_texture{};
    using Glyph = fnta::Glyph;
    using GlyphMap = FixedMapView<const char32_t, const Glyph>;
    GlyphMap m_glyphMap{};

    std::tuple<Glyph, Texture, math::vec2, uint32_t> getGlyph( char32_t ) const;

public:
    struct CreateInfo {
        std::span<const uint8_t> fontAtlas{};
        const Font* upstream = nullptr;
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
    math::vec2 extent() const;
    bool hasCodepoint( char32_t ) const;

    Sprite find( Hash::value_type ) const;
    Sprite find( char32_t ) const;

    struct RenderText {
        PushData pushData{};
        ui::PushConstant<ui::Pipeline::eSpriteSequence> pushConstant;
        math::vec2 extent;
    };
    RenderText composeText( const math::vec4& color, std::u32string_view, const math::vec2& geometry = math::vec2{ 320.0f, 100.0f } ) const;
    void appendRenderText( math::vec2&, PushData&, ui::PushConstant<ui::Pipeline::eSpriteSequence>&, char32_t ) const;
};

}
