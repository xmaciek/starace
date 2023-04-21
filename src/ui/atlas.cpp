#include <ui/atlas.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

Atlas::Sprite::operator math::vec4 () const noexcept
{
    return math::vec4{ x, y, w, h };
}

math::vec4 Atlas::Sprite::operator / ( const math::vec2& extent ) const noexcept
{
    return math::vec4{ x, y, w, h } / math::vec4{ extent.x, extent.y, extent.x, extent.y };
}



Atlas::Atlas( std::span<const std::tuple<hash_type, Sprite>> data, uint16_t width, uint16_t height ) noexcept
: m_width{ width }
, m_height{ height }
{
    assert( data.size() <= m_map.CAPACITY );
    assert( std::is_sorted( data.begin(), data.end(), &sortCmp ) );
    for ( auto&& [ hash, sprite ] : data ) {
        m_map.pushBack( hash, sprite );
    }
}

math::vec2 Atlas::extent() const
{
    return math::vec2{ m_width, m_height };
}


Atlas::Sprite Atlas::operator [] ( hash_type h ) const noexcept
{
    const Sprite* s = m_map[ h ];
    assert( s );
    return s ? *s : Sprite{};
}

}
