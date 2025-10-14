#pragma once

#include <ui/font.hpp>
#include <shared/hash.hpp>
#include <renderer/texture.hpp>

#include <memory_resource>
#include <vector>
#include <cstdint>

namespace ui {

class FontMap {
    std::pmr::vector<Font> m_fonts;

public:
    void addFont( std::span<const uint8_t> );
    const Font* findFont( Hash::value_type ) const;
    std::tuple<Font::Glyph, Texture, math::vec2, uint32_t> getGlyph( Hash::value_type name, char32_t ) const;
};

}
