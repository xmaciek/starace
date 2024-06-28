#include <ui/image.hpp>

#include <ui/atlas.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size }
, m_color{ g_uiProperty.color( ci.color ) }
{
    m_pipelineSlot = g_uiProperty.pipelineSpriteSequence();
    if ( ci.data ) {
        m_sampleRGBA = 1;
        m_dataModel = g_uiProperty.dataModel( ci.data );
        m_current = m_dataModel->current();
        setTexture( m_dataModel->texture( m_current ) );
    }
    else if ( ci.spriteId ) {
        m_sampleRGBA = 0;
        std::tie( m_uvwh, std::ignore, std::ignore, m_texture ) = g_uiProperty.sprite( ci.spriteId );
    }
    else {
        assert( !"expected data model or sprite id when creating image" );
    }
};

void Image::render( RenderContext rctx ) const
{
    assert( m_texture );
    PushBuffer pushBuffer{
        .m_pipeline = m_pipelineSlot,
        .m_verticeCount = 6,
    };
    pushBuffer.m_fragmentTexture[ 1 ] = m_texture;

    const math::vec2 pos = position() + offsetByAnchor();
    PushConstant<Pipeline::eSpriteSequence> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = m_color,
    };
    pushConstant.m_sprites[ 0 ].m_xywh = math::vec4{ pos.x, pos.y, m_size.x, m_size.y };
    pushConstant.m_sprites[ 0 ].m_uvwh = m_uvwh;
    pushConstant.m_sprites[ 0 ].m_sampleRGBA = m_sampleRGBA;
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
