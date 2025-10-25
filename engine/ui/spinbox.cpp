#include <ui/spinbox.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace {

ui::Sprite arrowLeft( math::vec2 size )
{
    auto arrow = g_uiProperty.sprite( "arrowLeft"_hash );
    arrow.x = static_cast<uint16_t>( size.x * 0.5f );
    arrow.y = static_cast<uint16_t>( size.y * 0.5f ) - ( arrow.h >> 1 );
    return arrow;
}

ui::Sprite arrowRight( math::vec2 size )
{
    auto arrow = g_uiProperty.sprite( "arrowRight"_hash );
    arrow.x = static_cast<uint16_t>( size.x - static_cast<float>( arrow.w ) - 4.0f );
    arrow.y = static_cast<uint16_t>( size.y * 0.5f - ( arrow.h >> 1 ) );
    return arrow;
}

}

namespace ui {

SpinBox::SpinBox( const CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .anchor = Anchor::fTop| Anchor::fLeft } }
, m_model{ g_uiProperty.dataModel( ci.data ) }
, m_arrowLeft{ arrowLeft( size() ) }
, m_arrowRight{ arrowRight( size() ) }
{
    m_label = emplace_child<Label>( Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .anchor = Anchor::fLeft | Anchor::fMiddle, } );
    m_value = emplace_child<Label>( Label::CreateInfo{ .data = ci.data, .font = "medium"_hash, .anchor = Anchor::fCenter | Anchor::fMiddle, } );
    math::vec2 s = size();
    m_label->setPosition( math::vec2{ 16.0f, s.y * 0.5f } );
    m_value->setPosition( math::vec2{ s.x * 0.75f - 2.0f, s.y * 0.5f } );
    setTabOrder( ci.tabOrder );
}

void SpinBox::render( const RenderContext& r ) const
{
    auto rctx = r;
    using PushConstant = PushConstant<Pipeline::eSpriteSequenceColors>;
    rctx.colorMain = isFocused() ? rctx.colorFocus : rctx.colorMain;
    NineSlice::render( rctx );

    RenderInfo ri{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = 2u,
    };
    const bool diffTex = m_arrowLeft.texture != m_arrowRight.texture;
    ri.m_fragmentTexture[ 0 ] = m_arrowLeft.texture;
    ri.m_fragmentTexture[ 1 ] = diffTex ? m_arrowRight.texture : Texture{};

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    pushConstant.m_sprites[ 0 ].m_color = m_focusL ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 0 ].m_xywh = m_arrowLeft.geometry();
    pushConstant.m_sprites[ 0 ].m_xywh.x += math::nonlerp( -5.0f, 0.0f, m_animL );
    pushConstant.m_sprites[ 0 ].m_uvwh = m_arrowLeft;
    pushConstant.m_sprites[ 0 ].m_whichAtlas = 0;
    pushConstant.m_sprites[ 0 ].m_sampleRGBA = rctx.renderer->channelCount( m_arrowLeft.texture ) == 4;

    pushConstant.m_sprites[ 1 ].m_color = m_focusR ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 1 ].m_xywh = m_arrowRight.geometry();
    pushConstant.m_sprites[ 1 ].m_xywh.x += math::nonlerp( 5.0f, 0.0f, m_animR );
    pushConstant.m_sprites[ 1 ].m_uvwh =  m_arrowRight;
    pushConstant.m_sprites[ 1 ].m_whichAtlas = diffTex;
    pushConstant.m_sprites[ 1 ].m_sampleRGBA = rctx.renderer->channelCount( m_arrowRight.texture ) == 4;

    ri.m_uniform = pushConstant;
    rctx.renderer->render( ri );
}

EventProcessing SpinBox::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    setFocused( testRect( p, pos, s ) );
    if ( !isFocused() ) {
        m_focusL = false;
        m_focusR = false;
        return EventProcessing::eContinue;
    }


    const bool left = testRect( p, m_arrowLeft.geometry() + math::vec4{ pos.x, pos.y, 0.0f, 0.0f } );
    const bool right = testRect( p, m_arrowRight.geometry() + math::vec4{ pos.x, pos.y, 0.0f, 0.0f } );
    switch ( event.type ) {
    case MouseEvent::eMove:
        m_focusL = left;
        m_focusR = right;
        break;
    case MouseEvent::eClick:
        if ( left ) {
            m_animL = 0.0f;
            m_model->select( static_cast<DataModel::size_type>( ui::overflow<int>( m_model->current() - 1, m_model->size() ) ) );
            break;
        }
        if ( right ) {
            m_animR = 0.0f;
            m_model->select( static_cast<DataModel::size_type>( ui::overflow<int>( m_model->current() + 1, m_model->size() ) ) );
            break;
        }
        break;
    default:
        break;
    }

    return EventProcessing::eStop;
}


EventProcessing SpinBox::onAction( ui::Action action )
{
    assert( m_model );
    if ( action.value == 0 ) { return EventProcessing::eContinue; }
    switch ( action.a ) {
    case ui::Action::eMenuLeft:
        m_animL = 0.0f;
        m_model->select( static_cast<DataModel::size_type>( ui::overflow<int>( m_model->current() - 1, m_model->size() ) ) );
        return EventProcessing::eStop;
    case ui::Action::eMenuRight:
        m_animR = 0.0f;
        m_model->select( static_cast<DataModel::size_type>( ui::overflow<int>( m_model->current() + 1, m_model->size() ) ) );
        return EventProcessing::eStop;
    default:
        return EventProcessing::eContinue;
    }
}

void SpinBox::update( const UpdateContext& uctx )
{
    m_animL = std::min( m_animL + uctx.deltaTime * 7.0f, 1.0f );
    m_animR = std::min( m_animR + uctx.deltaTime * 7.0f, 1.0f );
}


}
