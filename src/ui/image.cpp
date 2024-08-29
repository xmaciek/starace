#include <ui/image.hpp>

#include <ui/atlas.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <iostream>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_color{ g_uiProperty.color( ci.color ) }
{
    m_pipelineSlot = g_uiProperty.pipelineSpriteSequence();
    if ( ci.data ) {
        m_dataModel = g_uiProperty.dataModel( ci.data );
        m_revision = m_dataModel->revision();
        if ( auto tex = m_dataModel->texture( m_dataModel->current() ); tex ) {
            setTexture( tex );
        }
        else if ( Hash::value_type sprite = m_dataModel->sprite( m_dataModel->current() ); sprite ) {
            std::tie( m_uvwh, std::ignore, std::ignore, m_texture ) = g_uiProperty.sprite( sprite );
            setTexture( m_texture );
        }
    }
    else if ( ci.spriteId ) {
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
    DataModel::size_type rev = m_dataModel->revision();
    if ( rev == m_revision ) {
        return;
    }
    m_revision = rev;
    if ( Texture tex = m_dataModel->texture( m_dataModel->current() ); tex ) {
        m_uvwh = math::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
        setTexture( tex );
    }
    else if ( Hash::value_type sprite = m_dataModel->sprite( m_dataModel->current() ); sprite ) {
        std::tie( m_uvwh, std::ignore, std::ignore, m_texture ) = g_uiProperty.sprite( sprite );
        setTexture( m_texture );
    }
}

void Image::setColor( math::vec4 c )
{
    m_color = c;
}

void Image::setTexture( Texture t )
{
    assert( t );
    m_texture = t;
    m_sampleRGBA = Renderer::instance()->channelCount( t ) == 4;
}

}
