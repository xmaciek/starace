#include "progressbar.hpp"

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>
#include <ranges>

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
    using Uniform = PushConstant<Pipeline::eSpriteSequenceColors>;
    Uniform uniform{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    RenderInfo ri{
        .m_pipeline = m_pipeline,
        .m_verticeCount = Uniform::VERTICES,
    };
    ri.m_uniforms.emplace_back( uniform );
    ri.m_fragmentTexture.emplace_back( m_sprite.texture );

    auto makeInstance = [this]( uint32_t i )
    {
        return Uniform::Instance{
            .m_color = (float)i < ( m_value * (float)m_count ) ? winScreen : crimson,
            .m_xywh{ math::vec2{ m_spacing * i, 0.0f }, m_wh },
            .m_uvwh = m_sprite,
        };
    };
    auto submitBatch = [&rctx, &uniform, &ri, makeInstance]( auto&& batch )
    {
        std::ranges::transform( batch, uniform.m_instances.begin(), makeInstance );
        ri.m_instanceCount = static_cast<uint32_t>( batch.size() );
        rctx.renderer->render( ri );
    };
    auto batches = std::views::iota( 0u, m_count ) | std::views::chunk( uniform.INSTANCES );
    std::ranges::for_each( batches, submitBatch );
}

void Progressbar::update( const UpdateContext& )
{
    if ( !m_dataModel ) { return; }
    const auto rev = m_dataModel->revision();
    if ( rev == m_revision ) { return; }
    m_revision = rev;
    m_value = m_dataModel->data( m_dataModel->current() ).visit( GetFloat{} );
}

}
