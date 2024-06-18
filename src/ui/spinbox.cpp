#include <ui/spinbox.hpp>

#include <ui/property.hpp>
#include <ui/pipeline.hpp>
#include <ui/spritegen.hpp>

#include <renderer/renderer.hpp>

#include <cassert>

static constexpr std::array<Hash::value_type, 9> SLICES = {
    "topLeft"_hash,
    "top"_hash,
    "topRight"_hash,
    "left"_hash,
    "mid"_hash,
    "right"_hash,
    "botLeft"_hash,
    "bot"_hash,
    "botRight"_hash,
};

namespace ui {

SpinBox::SpinBox( const CreateInfo& ci ) noexcept
: Widget{ ci.position, ci.size }
, m_model{ g_uiProperty.dataModel( ci.data ) }
, m_index{ m_model->current(), 0, m_model->size() }
, m_label{ Label::CreateInfo{ .data = ci.data, .font = "small"_hash, .anchor = Anchor::fCenter | Anchor::fMiddle, } }
{
    setTabOrder( ci.tabOrder );
}

void SpinBox::render( RenderContext rctx ) const
{
    PushData pushData{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = 6u,
        .m_instanceCount = 11u,
    };
    pushData.m_resource[ 1 ].texture = g_uiProperty.atlasTexture();

    PushConstant<Pipeline::eSpriteSequenceColors> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    math::vec4 left = arrowLeft();
    math::vec4 right = arrowRight();
    math::vec4 mid{ left.x + left.z, pos.y, s.x - ( left.z + right.z ), s.y };

    auto nonlerp2 = []( float a, float b, float n )
    {
        n = 0.5f - math::cos( n * math::pi * 2.0f ) * 0.5f;
        return math::lerp( a, b, n );
    };
    left.x += nonlerp2( 0.0f, -5.0f, m_animL );
    right.x += nonlerp2( 0.0f, 5.0f, m_animR );

    const auto& atlasRef = *g_uiProperty.atlas();
    math::vec2 altasExtent = atlasRef.extent();
    pushConstant.m_sprites[ 0 ].m_color = m_focusL ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 0 ].m_xywh = left;
    pushConstant.m_sprites[ 0 ].m_uvwh = atlasRef[ "arrowLeft"_hash ] / altasExtent;

    pushConstant.m_sprites[ 1 ].m_color = m_focusR ? rctx.colorFocus : rctx.colorMain;
    pushConstant.m_sprites[ 1 ].m_xywh = right;
    pushConstant.m_sprites[ 1 ].m_uvwh =  atlasRef[ "arrowRight"_hash ] / altasExtent;

    auto color = isFocused() ? rctx.colorFocus : rctx.colorMain;
    NineSlice2 gen{ mid, g_uiProperty.atlas(), SLICES };
    for ( auto i = 0u; i < 9u; ++i ) {
        auto& sprite = pushConstant.m_sprites[ i + 2u ];
        sprite.m_color = color;
        sprite.m_xywh = gen( i );
        sprite.m_uvwh = atlasRef[ SLICES[ i ] ] / altasExtent;
    }
    rctx.renderer->push( pushData, &pushConstant );

    const math::vec2 mv = pos + s * 0.5f;
    rctx.model = math::translate( rctx.model, math::vec3{ mv.x, mv.y, 0.0f } );
    m_label.render( rctx );
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


    const bool left = testRect( p, arrowLeft() );
    const bool right = testRect( p, arrowRight() );
    switch ( event.type ) {
    case MouseEvent::eMove:
        m_focusL = left;
        m_focusR = right;
        break;
    case MouseEvent::eClick:
        if ( left ) {
            m_animL = 0.0f;
            m_index--;
            m_model->select( value() );
            break;
        }
        if ( right ) {
            m_animR = 0.0f;
            m_index++;
            m_model->select( value() );
            break;
        }
        m_model->activate( value() );
        break;
    default:
        break;
    }

    return EventProcessing::eStop;
}

math::vec4 SpinBox::arrowLeft() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    static constexpr auto arrowHash = "arrowLeft"_hash;
    auto arrow = (*g_uiProperty.atlas())[ arrowHash ];
    return math::vec4{ pos.x, pos.y, arrow.w, arrow.h };
}

math::vec4 SpinBox::arrowRight() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    static constexpr auto arrowHash = "arrowRight"_hash;
    auto arrow = (*g_uiProperty.atlas())[ arrowHash ];
    return math::vec4{ pos.x + s.x - static_cast<float>( arrow.w ), pos.y, arrow.w, arrow.h };
}

DataModel::size_type SpinBox::value() const
{
    assert( m_model );
    assert( *m_index < m_model->size() );
    return *m_index;
}

EventProcessing SpinBox::onAction( ui::Action action )
{
    if ( action.value == 0 ) { return EventProcessing::eContinue; }
    switch ( action.a ) {
    case ui::Action::eMenuLeft:
        m_animL = 0.0f;
        m_index--;
        m_model->select( value() );
        break;
    case ui::Action::eMenuRight:
        m_animR = 0.0f;
        m_index++;
        m_model->select( value() );
        break;
    case ui::Action::eMenuConfirm:
        m_model->activate( value() );
        break;
    default:
        return EventProcessing::eContinue;
    }
    m_label.setText( m_model->at( value() ) );
    return EventProcessing::eStop;
}

void SpinBox::update( const UpdateContext& uctx )
{
    m_label.update( uctx );
    m_animL = std::min( m_animL + uctx.deltaTime * 7.0f, 1.0f );
    m_animR = std::min( m_animR + uctx.deltaTime * 7.0f, 1.0f );
}


}
