#include "spritegen.hpp"
#include "linear_atlas.hpp"


#include <cassert>

namespace spritegen {

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

math::vec4 NineSliceComposer::operator () () noexcept
{
    assert( m_atlas );
    assert( m_currentVert < count() );

    const uint32_t currentVert = m_currentVert++;
    const uint32_t spriteId = currentVert / 6u;
    const uint32_t vertId = currentVert % 6u;
    const uint32_t row = spriteId / 3u;
    const uint32_t column = spriteId % 3u;

    const Sprite sprite = m_atlas->sprite( m_spriteIds[ spriteId ] );
    const math::vec4 uvwh = m_atlas->sliceUV( m_spriteIds[ spriteId ] );
    const float w = ( column == 1u ) ? m_midStretch.x : static_cast<float>( sprite[ 2u ] );
    const float h = ( row == 1u )    ? m_midStretch.y : static_cast<float>( sprite[ 3u ] );
    const math::vec4 xywh = m_xyBegin + m_xyOffset + math::vec4{ 0.0f, 0.0f, w, h };

    if ( vertId == 5u ) {
        if ( spriteId % 3u == 2u ) {
            m_xyOffset.x = 0.0f;
            m_xyOffset.y += h;
        }
        else {
            m_xyOffset.x += w;
        }
    }

    return xyuvForVertice6( xywh, uvwh, vertId );

}

NineSlice2::NineSlice2( const math::vec4& xywh, const LinearAtlas* atlas, const std::array<uint32_t, 9>& ids ) noexcept
: m_xy{ xywh.x, xywh.y }
, m_atlas{ atlas }
, m_spriteIds{ ids }
{
    Sprite s0 = atlas->sprite( m_spriteIds[ 0u ] );
    Sprite s2 = atlas->sprite( m_spriteIds[ 2u ] );
    Sprite s6 = atlas->sprite( m_spriteIds[ 6u ] );

    m_w[ 0 ] = s0[ 2 ];
    m_w[ 1 ] = xywh.z - ( s0[ 2 ] + s2[ 2 ] );
    m_w[ 2 ] = s2[ 2 ];

    m_h[ 0 ] = s0[ 3 ];
    m_h[ 1 ] = xywh.w - ( s0[ 3 ] + s6[ 3 ] );
    m_h[ 2 ] = s6[ 3 ];
}

math::vec4 NineSlice2::operator () () noexcept
{
    assert( m_atlas );
    assert( m_currentVert < count() );

    const uint32_t currentVert = m_currentVert++;
    const uint32_t spriteId = currentVert / 6u;
    const uint32_t vertId = currentVert % 6u;

    const math::vec4 uvwh = m_atlas->sliceUV( m_spriteIds[ spriteId ] );

    math::vec4 xywh{};
    switch ( spriteId ) {
    case 0: xywh = math::vec4{ m_xy.x,                       m_xy.y, m_w[ 0 ], m_h[ 0 ] }; break;
    case 1: xywh = math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y, m_w[ 1 ], m_h[ 0 ] }; break;
    case 2: xywh = math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y, m_w[ 2 ], m_h[ 0 ] }; break;

    case 3: xywh = math::vec4{ m_xy.x,                       m_xy.y + m_h[ 0 ], m_w[ 0 ], m_h[ 1 ] }; break;
    case 4: xywh = math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y + m_h[ 0 ], m_w[ 1 ], m_h[ 1 ] }; break;
    case 5: xywh = math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y + m_h[ 0 ], m_w[ 2 ], m_h[ 1 ] }; break;

    case 6: xywh = math::vec4{ m_xy.x,                       m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 0 ], m_h[ 2 ] }; break;
    case 7: xywh = math::vec4{ m_xy.x + m_w[ 0 ],            m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 1 ], m_h[ 2 ] }; break;
    case 8: xywh = math::vec4{ m_xy.x + m_w[ 0 ] + m_w[ 1 ], m_xy.y + m_h[ 0 ] + m_h[ 1 ], m_w[ 2 ], m_h[ 2 ] }; break;
    }

    return xyuvForVertice6( xywh, uvwh, vertId );

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
