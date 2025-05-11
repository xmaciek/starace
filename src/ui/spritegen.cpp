#include <ui/spritegen.hpp>

#include <cassert>

static math::vec4 xyuvForVertice6( const math::vec4& xywh, const math::vec4& uvwh, uint32_t vert )
{
    switch ( vert ) {
    case 0: [[fallthrough]];
    case 5: return math::vec4{ xywh.x,          xywh.y,          uvwh.x,          uvwh.y };
    case 1: return math::vec4{ xywh.x,          xywh.y + xywh.w, uvwh.x,          uvwh.y + uvwh.w };

    case 2: [[fallthrough]];
    case 3: return math::vec4{ xywh.x + xywh.z, xywh.y + xywh.w, uvwh.x + uvwh.z, uvwh.y + uvwh.w };
    case 4: return math::vec4{ xywh.x + xywh.z, xywh.y,          uvwh.x + uvwh.z, uvwh.y };
    default:
        assert( !"vertice id out of bounds" );
        return {};
    }
}

namespace ui {

NineSlice2::NineSlice2( const math::vec4& xywh, const Font* atlas, const std::array<Hash::value_type, 9>& ids ) noexcept
: m_xy{ xywh.x, xywh.y }
, m_atlas{ atlas }
, m_spriteIds{ ids }
{
    assert( atlas );
    auto s0 = atlas->find( m_spriteIds[ 0u ] );
    auto s2 = atlas->find( m_spriteIds[ 2u ] );
    auto s6 = atlas->find( m_spriteIds[ 6u ] );

    m_w[ 0 ] = s0.w;
    m_w[ 1 ] = xywh.z - ( s0.w + s2.w );
    m_w[ 2 ] = s2.w;

    m_h[ 0 ] = s0.h;
    m_h[ 1 ] = xywh.w - ( s0.h + s6.h );
    m_h[ 2 ] = s6.h;
}

math::vec4 NineSlice2::operator () () noexcept
{
    assert( m_atlas );
    assert( m_currentVert < count() );

    const uint32_t currentVert = m_currentVert++;
    const uint32_t spriteId = currentVert / 6u;
    const uint32_t vertId = currentVert % 6u;

    const math::vec4 uvwh = m_atlas->find( m_spriteIds[ spriteId ] );

    math::vec4 xywh = (*this)( spriteId );

    return xyuvForVertice6( xywh, uvwh, vertId );

}

math::vec4 NineSlice2::operator () ( uint32_t spriteId ) const noexcept
{
    switch ( spriteId ) {
    case 0: return math::vec4{ m_xy.x,                       m_xy.y, m_w[ 0 ], m_h[ 0 ] }; break;
    case 1: return math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y, m_w[ 1 ], m_h[ 0 ] }; break;
    case 2: return math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y, m_w[ 2 ], m_h[ 0 ] }; break;

    case 3: return math::vec4{ m_xy.x,                       m_xy.y + m_h[ 0 ], m_w[ 0 ], m_h[ 1 ] }; break;
    case 4: return math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y + m_h[ 0 ], m_w[ 1 ], m_h[ 1 ] }; break;
    case 5: return math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y + m_h[ 0 ], m_w[ 2 ], m_h[ 1 ] }; break;

    case 6: return math::vec4{ m_xy.x,                       m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 0 ], m_h[ 2 ] }; break;
    case 7: return math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 1 ], m_h[ 2 ] }; break;
    case 8: return math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 2 ], m_h[ 2 ] }; break;

    default:
        assert( !"sprite id out of bounds" );
        return {};
    }

}

math::vec4 Vert6::operator () () noexcept
{
    assert( m_currentVert < count() );
    switch ( m_currentVert++ ) {
    case 0: [[fallthrough]];
    case 5: return math::vec4{ m_xywh.x, m_xywh.y,                          m_uvwh.x, m_uvwh.y };
    case 1: return math::vec4{ m_xywh.x, m_xywh.y + m_xywh.w,               m_uvwh.x, m_uvwh.y + m_uvwh.w };
    case 2: [[fallthrough]];
    case 3: return math::vec4{ m_xywh.x + m_xywh.z, m_xywh.y + m_xywh.w,    m_uvwh.x + m_uvwh.z, m_uvwh.y + m_uvwh.w };
    case 4: return math::vec4{ m_xywh.x + m_xywh.z, m_xywh.y,               m_uvwh.x + m_uvwh.z, m_uvwh.y };
    default:
        assert( !"vertice id out of bounds" );
        return {};
    }
}




}
