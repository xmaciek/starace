#include "ui_generator.hpp"
#include "linear_atlas.hpp"


#include <cassert>

namespace ui::generator {

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






}
