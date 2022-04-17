#include "ui_image.hpp"

#include "game_pipeline.hpp"
#include "ui_data_model.hpp"

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
    const PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eGuiTextureColor1 ),
        .m_verticeCount = 4,
        .m_texture = m_texture,
    };

    PushConstant<Pipeline::eGuiTextureColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_view = rctx.view;
    pushConstant.m_color = m_color;

    const math::vec2 pos = position() + offsetByAnchor();
    const float x = pos.x;
    const float y = pos.y;
    const float xw = x + m_size.x;
    const float yh = y + m_size.y;
    pushConstant.m_vertices[ 0 ] = math::vec4{ x, y, 0.0f, 0.0f };
    pushConstant.m_vertices[ 1 ] = math::vec4{ x, yh, 0.0f, 1.0f };
    pushConstant.m_vertices[ 2 ] = math::vec4{ xw, yh, 1.0f, 1.0f };
    pushConstant.m_vertices[ 3 ] = math::vec4{ xw, y, 1.0f, 0.0f };

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
