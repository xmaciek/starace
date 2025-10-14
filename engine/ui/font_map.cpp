#include <ui/font_map.hpp>

#include <shared/max_score_element.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

void FontMap::addFont( std::span<const uint8_t> data )
{
    m_fonts.emplace_back( Font::CreateInfo{
        .fontAtlas = data,
    } );
}

const Font* FontMap::findFont( Hash::value_type hash ) const
{
    auto it = std::ranges::find_if( m_fonts, [hash]( const Font& f ) { return f.m_name == hash; } );
    assert( it != m_fonts.end() );
    return &*it;
}

std::tuple<Font::Glyph, Texture, math::vec2, uint32_t> FontMap::getGlyph( Hash::value_type name, char32_t codepoint ) const
{
    auto it = maxScoreElement( m_fonts, [name, codepoint]( const auto& f ) -> size_t
    {
        if ( !f.hasCodepoint( codepoint ) ) return 0;
        return ( f.m_name == name ) * 10'000'000u + f.m_lineHeight;
    } );
    assert( it != m_fonts.end() );
    auto g = it->m_glyphMap.find( codepoint );
    assert( g );
    if ( g ) [[likely]] return std::make_tuple( *g, it->m_texture, it->extent(), it->m_lineHeight );
    return {};
}

}
