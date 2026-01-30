#include "progressbar.hpp"

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

namespace ui {

Progressbar::Progressbar( const Progressbar::CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_count{ ci.count }
{
    m_pipeline = g_uiProperty.findMaterial( "spriteSequenceColors"_hash );
    m_dataModel = g_uiProperty.dataModel( ci.data );
    m_sprite = g_uiProperty.sprite( ci.path );
    float aspect = size().y / (float)m_sprite.h;
    m_wh = math::vec2{ (float)m_sprite.w * aspect, size().y };
    m_spacing = m_wh.x + ci.spriteSpacing * aspect;
    if ( ci.count == 0 ) {
        m_count = (uint16_t)( size().x / m_spacing );
    }
    assert( m_count );
}

static const math::vec4 winScreen{ 0.0275f, 1.0f, 0.075f, 1.0f };
static const math::vec4 crimson{ 0.863f, 0.078f,  0.234f, 1.0f };

void Progressbar::render( const RenderContext& rctx ) const
{
    using Instanced = InstancedRendering<PushConstant<Pipeline::eSpriteSequenceColors>>;
    Instanced instanced{ rctx.renderer, m_pipeline };
    instanced.renderInfo.m_fragmentTexture[ 0 ] = m_sprite.texture;
    instanced.pushConstant.m_model = rctx.model;
    instanced.pushConstant.m_view = rctx.view;
    instanced.pushConstant.m_projection = rctx.projection;

    for ( auto i = m_count; i--; ) {
        instanced.append( Instanced::Instance{
            .m_color = (float)i < ( m_value * (float)m_count ) ? winScreen : crimson,
            .m_xywh{ math::vec2{ m_spacing * i, 0.0f }, m_wh },
            .m_uvwh = m_sprite,
        } );
    }
}

void Progressbar::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto rev = m_dataModel->revision();
    if ( rev == m_revision ) { return; }
    m_revision = rev;
    m_value = m_dataModel->atF( m_dataModel->current() );
}

}
