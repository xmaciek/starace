#include <ui/property.hpp>
#include <ui/font.hpp>

namespace ui {

Sprite Property::sprite( Hash::value_type hash ) const
{
    return m_sprites[ hash ];
}

void Property::addSprites( const Font* font )
{
    assert( font );

    math::vec4 div{ font->m_width, font->m_height, font->m_width, font->m_height };
    auto convert = [this, div, t=font->m_texture, v=font->m_glyphMap.m_values.begin()]( char32_t c ) mutable
    {
        auto&& g = *v; v++;
        Sprite sprite{
            .texture = t,
            .x = g.position[ 0 ],
            .y = g.position[ 1 ],
            .w = g.size[ 0 ],
            .h = g.size[ 1 ],
        };
        sprite.xyuv = math::vec4{ sprite.x, sprite.y, sprite.w, sprite.h } / div;
        m_sprites.insert( std::make_pair( static_cast<Hash::value_type>( c ), sprite ) );
    };
    std::ranges::for_each( font->m_glyphMap.m_keys, convert );
}

void Property::changeScreen( Hash::value_type hash, math::vec2 viewport )
{
    auto it = std::ranges::find_if( m_screens, [hash]( const auto& sc ) { return sc.name() == hash; } );
    assert( it != m_screens.end() );
    m_viewport = viewport;
    m_currentScreen = &*it;
    m_currentScreen->show( viewport );
}

void Property::changeScreen( Hash::value_type hash )
{
    changeScreen( hash, m_viewport );
}

uint32_t Property::changeLockit( std::array<char, 8> id )
{
    auto it = std::ranges::find_if( m_lockit, [id]( const auto& l ) { return l.id() == id; } );
    assert( it != m_lockit.end() );
    m_currentLang = static_cast<uint32_t>( std::distance( m_lockit.begin(), it ) );
    std::ranges::for_each( m_screens, []( auto& s ) { s.lockitChanged(); } );
    return m_currentLang;
}

void Property::loadFNTA( std::span<const uint8_t> data )
{
    m_fontMap.addFont( data );
}

void Property::loadATLAS( std::span<const uint8_t> data )
{
    assert( m_textures );
    ui::Font f = ui::Font::CreateInfo{
        .fontAtlas = data,
    };
    addSprites( &f );
}

void Property::loadUI( std::span<const uint8_t> data )
{
    m_screens.emplace_back( data );
}

}
