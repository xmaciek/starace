#include "ui_image.hpp"

#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Image::Image( Texture t )
: m_texture{ t }
{
    assert( t );
};

Image::Image( Texture t, math::vec4 color )
: m_color{ color }
, m_texture{ t }
{
    assert( t );
};

Image::Image( math::vec2 position, math::vec2 extent, math::vec4 color, Texture t )
: Widget{ position, extent }
, m_color{ color }
, m_texture{ t }
{
    assert( t );
}

Image::Image( Anchor a, math::vec2 extent, math::vec4 color, Texture t )
: Widget{ {}, extent, a }
, m_color{ color }
, m_texture{ t }
{
    assert( t );
}

Image::Image( Anchor a )
: Widget{ a }
{
}

Image::Image( math::vec2 position, math::vec2 extent, DataModel* dataModel )
: Widget{ position, extent }
, m_dataModel{ dataModel }
{
    assert( dataModel );
    m_current = m_dataModel->current();
    setTexture( dataModel->texture( m_current ) );
}

void Image::render( RenderContext rctx ) const
{
    assert( m_texture );
    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eSpriteSequenceRGBA ],
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    PushConstant<Pipeline::eSpriteSequenceRGBA> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };
    pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ pos.x, pos.y, m_size.x, m_size.y };
    pushConstant.m_sprites[ 0 ].m_uvwh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };

    rctx.renderer->push( pushBuffer, &pushConstant );
}

void Image::update( const UpdateContext& )
{
    if ( !m_dataModel ) {
        return;
    }
    DataModel::size_type idx = m_dataModel->current();
    if ( idx == m_current ) {
        return;
    }
    m_current = idx;
    setTexture( m_dataModel->texture( m_current ) );
}

void Image::setColor( math::vec4 c )
{
    m_color = c;
}

void Image::setTexture( Texture t )
{
    assert( t );
    m_texture = t;
}

}
