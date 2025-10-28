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

void Property::loadFNTA( std::span<const uint8_t> data )
{
    g_uiProperty.m_fontMap.addFont( data );
}

void Property::loadATLAS( std::span<const uint8_t> data )
{
    assert( m_textures );
    ui::Font f = ui::Font::CreateInfo{
        .fontAtlas = data,
    };
    g_uiProperty.addSprites( &f );
}

}
