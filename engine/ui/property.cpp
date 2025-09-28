#include <ui/property.hpp>
#include <ui/font.hpp>

namespace ui {

Sprite Property::sprite( Hash::value_type hash ) const
{
    return m_sprites[ hash ];
}


Texture Property::atlasTexture() const
{
    return m_atlas->texture();
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

std::function<void(std::pair<uint32_t, PipelineSlot>)> Property::setupPipeline()
{
    return [this]( std::pair<uint32_t, PipelineSlot> p )
    {
        switch ( static_cast<Pipeline>( p.first ) ) {
        case eSpriteSequence: m_pipelineSpriteSequence = p.second; break;
        case eSpriteSequenceColors: m_pipelineSpriteSequenceColors = p.second; break;
        case eGlow: m_pipelineGlow = p.second; break;
        case eBlurDesaturate: m_pipelineBlurDesaturate = p.second; break;
        default:
            assert( !"unhandled enum for ui pipelines" );
        };
    };
}

}
