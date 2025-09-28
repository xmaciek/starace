#include <ui/spinbox.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <ui/spritegen.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

namespace ui {

SpinBox::SpinBox( const CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .anchor = Anchor::fTop| Anchor::fLeft } }
, m_model{ g_uiProperty.dataModel( ci.data ) }
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

    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = 2u,
    };
    pushData.m_fragmentTexture[ 0 ] = g_uiProperty.atlasTexture();

    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    math::vec4 left = arrowLeft();
    math::vec4 right = arrowRight();

    auto nonlerp2 = []( float a, float b, float n )
    {
        n = 0.5f - math::cos( n * math::pi * 2.0f ) * 0.5f;
        return math::lerp( a, b, n );
    };
    left.x += nonlerp2( 0.0f, -5.0f, m_animL );
    right.x += nonlerp2( 0.0f, 5.0f, m_animR );

    pushConstant.m_sprites[ 0 ].m_color = m_focusL ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 0 ].m_xywh = left;
    pushConstant.m_sprites[ 0 ].m_uvwh = g_uiProperty.sprite( "arrowLeft"_hash );

    pushConstant.m_sprites[ 1 ].m_color = m_focusR ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 1 ].m_xywh = right;
    pushConstant.m_sprites[ 1 ].m_uvwh =  g_uiProperty.sprite( "arrowRight"_hash );

    rctx.renderer->push( pushData, &pushConstant );
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


    const bool left = testRect( p, arrowLeft() + math::vec4{ pos.x, pos.y, 0.0f, 0.0f } );
    const bool right = testRect( p, arrowRight() + math::vec4{ pos.x, pos.y, 0.0f, 0.0f } );
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

math::vec4 SpinBox::arrowLeft() const
{
    const math::vec2 s = size();
    static constexpr auto arrowHash = "arrowLeft"_hash;
    auto arrow = g_uiProperty.sprite( arrowHash );
    return math::vec4{
        s.x * 0.5f,
        s.y * 0.5f - static_cast<float>( arrow.h ) * 0.5f,
        arrow.w,
        arrow.h
    };
}

math::vec4 SpinBox::arrowRight() const
{
    const math::vec2 s = size();
    static constexpr auto arrowHash = "arrowRight"_hash;
    auto arrow = g_uiProperty.sprite( arrowHash );
    return math::vec4{
        s.x - static_cast<float>( arrow.w ) - 4.0f,
        s.y * 0.5f - static_cast<float>( arrow.h ) * 0.5f,
        arrow.w,
        arrow.h
    };
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
