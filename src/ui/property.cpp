#include <ui/property.hpp>
#include <ui/font.hpp>

namespace ui {

Sprite Property::sprite( Hash::value_type hash ) const
{
    assert( m_atlas );
    return m_atlas->find( hash );
}


Texture Property::atlasTexture() const
{
    return m_atlas->texture();
}

}
