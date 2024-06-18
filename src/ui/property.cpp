#include <ui/property.hpp>
#include <ui/font.hpp>

namespace ui {

std::tuple<math::vec4, uint16_t, uint16_t, Texture> Property::sprite( Hash::value_type hash ) const
{
    Font::Sprite spr = (*m_atlas)[ hash ];
    return std::make_tuple( spr / m_atlas->extent(), spr.w, spr.h, m_atlas->texture() );
}


Texture Property::atlasTexture() const
{
    return m_atlas->texture();
}

}
