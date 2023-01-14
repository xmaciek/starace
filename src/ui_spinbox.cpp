#include "ui_spinbox.hpp"

#include "ui_property.hpp"
#include "spritegen.hpp"
#include "game_pipeline.hpp"
#include "linear_atlas.hpp"
#include "colors.hpp"

#include <renderer/renderer.hpp>

#include <cassert>

static constexpr std::array<uint32_t, 9> c_slices = {
    ui::AtlasSprite::eTopLeft,
    ui::AtlasSprite::eTop,
    ui::AtlasSprite::eTopRight,
    ui::AtlasSprite::eLeft,
    ui::AtlasSprite::eMid,
    ui::AtlasSprite::eRight,
    ui::AtlasSprite::eBotLeft,
    ui::AtlasSprite::eBot,
    ui::AtlasSprite::eBotRight,
};

namespace ui {

SpinBox::SpinBox( DataModel* dataModel ) noexcept
: Widget{ {}, { 240.0f, 48.0f } }
, m_index{ dataModel->current(), 0, dataModel->size() }
, m_model{ dataModel }
, m_label{ g_uiProperty.fontSmall(), Anchor::fCenter | Anchor::fMiddle, color::white }
{
    m_label.setText( m_model->at( value() ) );
}

void SpinBox::render( RenderContext rctx ) const
{
    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eSpriteSequenceColors ],
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

    pushConstant.m_sprites[ 0 ].m_color = m_focusL ? rctx.colorFocus : rctx.colorMain;
    std::generate_n( pushConstant.m_sprites[ 0 ].m_xyuv.begin(), 6, spritegen::Vert6{
        .m_xywh = left,
        .m_uvwh = g_uiProperty.atlas()->sliceUV( ui::AtlasSprite::eArrowLeft ),
    } );

    pushConstant.m_sprites[ 1 ].m_color = m_focusR ? rctx.colorFocus : rctx.colorMain;
    std::generate_n( pushConstant.m_sprites[ 1 ].m_xyuv.begin(), 6, spritegen::Vert6{
        .m_xywh = right,
        .m_uvwh = g_uiProperty.atlas()->sliceUV( ui::AtlasSprite::eArrowRight ),
    } );


    spritegen::NineSlice2 vertGen{ mid, g_uiProperty.atlas(), c_slices };
    auto gen = [&vertGen]() { return vertGen(); };

    for ( uint32_t i = 2; i < 11; ++i ) {
        pushConstant.m_sprites[ i ].m_color = isFocused() ? rctx.colorFocus : rctx.colorMain;
        std::generate_n( pushConstant.m_sprites[ i ].m_xyuv.begin(), 6, gen );
    };
    rctx.renderer->push( pushData, &pushConstant );

    const math::vec2 mv = pos + s * 0.5f;
    rctx.model = math::translate( rctx.model, math::vec3{ mv.x, mv.y, 0.0f } );
    m_label.render( rctx );
}

MouseEvent::Processing SpinBox::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    setFocused( testRect( p, pos, s ) );
    if ( !isFocused() ) {
        m_focusL = false;
        m_focusR = false;
        return MouseEvent::eContinue;
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
            m_label.setText( m_model->at( value() ) );
            break;
        }
        if ( right ) {
            m_animR = 0.0f;
            m_index++;
            m_model->select( value() );
            m_label.setText( m_model->at( value() ) );
            break;
        }
        m_model->activate( value() );
        break;
    default:
        break;
    }

    return MouseEvent::eStop;
}

math::vec4 SpinBox::arrowLeft() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    Sprite arrow = g_uiProperty.atlas()->sprite( ui::AtlasSprite::eArrowLeft );
    return { pos.x, pos.y, arrow[ 2 ], arrow[ 3 ] };
}

math::vec4 SpinBox::arrowRight() const
{
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    Sprite arrow = g_uiProperty.atlas()->sprite( ui::AtlasSprite::eArrowRight );
    return { pos.x + s.x - static_cast<float>( arrow[ 2 ] ), pos.y, arrow[ 2 ], arrow[ 3 ] };
}

DataModel::size_type SpinBox::value() const
{
    assert( m_model );
    assert( *m_index < m_model->size() );
    return *m_index;
}

bool SpinBox::onAction( Action a )
{
    if ( !a.digital ) { return false; }
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuLeft:
        m_animL = 0.0f;
        m_index--;
        m_model->select( value() );
        break;
    case GameAction::eMenuRight:
        m_animR = 0.0f;
        m_index++;
        m_model->select( value() );
        break;
    case GameAction::eMenuConfirm:
        m_model->activate( value() );
        break;
    default:
        return false;
    }
    m_label.setText( m_model->at( value() ) );
    return true;
}

void SpinBox::update( const UpdateContext& uctx )
{
    m_animL = std::min( m_animL + uctx.deltaTime * 7.0f, 1.0f );
    m_animR = std::min( m_animR + uctx.deltaTime * 7.0f, 1.0f );
}


}
