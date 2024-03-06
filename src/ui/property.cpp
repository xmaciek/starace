#include <ui/property.hpp>
#include <ui/atlas.hpp>

namespace ui {

std::tuple<math::vec4, uint16_t, uint16_t, Texture> Property::sprite( Hash::value_type hash ) const
{
    Atlas::Sprite spr = (*atlas())[ hash ];
    return std::make_tuple( spr / atlas()->extent(), spr.w, spr.h, atlasTexture() );
}

}
