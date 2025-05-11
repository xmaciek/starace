#include <ui/progressbar.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

namespace ui {

Progressbar::Progressbar( const Progressbar::CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size, ci.anchor }
, m_spacing{ ci.spriteSpacing }
, m_count{ ci.count }
{
    m_dataModel = g_uiProperty.dataModel( ci.data );
    auto sprite = g_uiProperty.sprite( ci.spriteId );
    m_uvwh = sprite;
    m_texture = sprite;
    m_spriteSize = { sprite.w, sprite.h };
    auto s = size();
    float aspectRatio = s.y / m_spriteSize.y;
    m_spriteSize *= aspectRatio;
    m_spacing *= aspectRatio;
    if ( ci.count == 0 ) {
        m_count = (uint16_t)( s.x / ( m_spriteSize.x + m_spacing ) );
    }
    assert( m_count );
}

void Progressbar::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eSpriteSequenceColors>;
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = m_count,
    };
    pushData.m_fragmentTexture[ 1 ] = m_texture;

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    const float xOffset = m_spriteSize.x + m_spacing;
    auto Gen = [this, pos = math::vec2{}, i = 0u, value = m_value, xOffset]() mutable
    {
        const math::vec4 winScreen{ 0.0275f, 1.0f, 0.075f, 1.0f };
        const math::vec4 crimson{ 0.863f, 0.078f,  0.234f, 1.0f };
        auto j = i++;
        return PushConstant::Sprite{
            .m_color = (float)j < ( value * (float)m_count ) ? winScreen : crimson,
            .m_xywh{ pos.x + ( j * xOffset ), pos.y, m_spriteSize.x, m_spriteSize.y },
            .m_uvwh = m_uvwh,
        };
    };
    std::generate_n( pushConstant.m_sprites.begin(), m_count, Gen );

    rctx.renderer->push( pushData, &pushConstant );
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
