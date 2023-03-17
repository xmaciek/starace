#include <ui/linear_atlas.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

LinearAtlas::LinearAtlas( std::span<const Sprite> data, uint16_t width, uint16_t height ) noexcept
: m_width{ width }
, m_height{ height }
{
    assert( data.size() <= m_sprites.size() );
    std::copy_n( data.begin(), std::min( data.size(), m_sprites.size() ), m_sprites.begin() );
}

Sprite LinearAtlas::sprite( uint32_t s ) const
{
    assert( s < m_sprites.size() );
    return m_sprites[ s ];
}

math::vec2 LinearAtlas::extent() const
{
    return math::vec2{ m_width, m_height };
}

math::vec4 LinearAtlas::sliceUV( uint32_t s ) const
{
    assert( s < m_sprites.size() );
    const Sprite sprite = m_sprites[ s ];
    const math::vec2 wh = extent();
    const float x = static_cast<float>( sprite[ 0 ] ) / wh.x;
    const float y = static_cast<float>( sprite[ 1 ] ) / wh.y;
    const float w = static_cast<float>( sprite[ 2 ] ) / wh.x;
    const float h = static_cast<float>( sprite[ 3 ] ) / wh.y;
    return math::vec4{ x, y, w, h };
}

}
