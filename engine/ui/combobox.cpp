#include <ui/combobox.hpp>

#include <ui/font.hpp>
#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <algorithm>
#include <cassert>
#include <string_view>

namespace ui {

class ComboBoxList : public Widget {
    DataModel* m_model = nullptr;
    ScrollIndex m_index{};
    Sprite m_hover{};
    float m_lineHeight = 0.0f;
    float m_topPadding = 0.0f;
    float m_botHeight = 8.0f;

    DataModel::size_type visibleCount() const;
    DataModel::size_type selectedIndex() const;

public:
    struct CreateInfo {
        DataModel* model = nullptr;
        math::vec2 position{};
        math::vec2 size{};
    };


    ~ComboBoxList() noexcept = default;
    ComboBoxList() noexcept = default;
    ComboBoxList( const CreateInfo& p ) noexcept;


    virtual void render( const RenderContext& ) const override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
};

ComboBox::ComboBox( const ComboBox::CreateInfo& ci ) noexcept
: NineSlice{ NineSlice::CreateInfo{ .position = ci.position, .size = ci.size, .anchor = Anchor::fTop| Anchor::fLeft } }
, m_data{ ci.data }
{
    m_label = emplace_child<Label>( Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .anchor = Anchor::fLeft | Anchor::fMiddle, } );
    m_value = emplace_child<Label>( Label::CreateInfo{ .data = ci.data, .font = ci.font, .anchor = Anchor::fRight | Anchor::fMiddle, } );
    math::vec2 s = size();
    m_label->setPosition( math::vec2{ 16.0f, s.y * 0.5f } );
    m_value->setPosition( math::vec2{ s.x - 16.0f, s.y * 0.5f } );
    setTabOrder( ci.tabOrder );
}

void ComboBox::open()
{
    g_uiProperty.requestModalWidget( UniquePointer<ComboBoxList>(
        std::pmr::get_default_resource(),
        ComboBoxList::CreateInfo{
            .model = g_uiProperty.dataModel( m_data ),
            .position = position() + offsetByAnchor(),
            .size = size(),
        }
    ) );
}

EventProcessing ComboBox::onAction( ui::Action action )
{
    switch ( action.a ) {
    case ui::Action::eMenuConfirm:
        open();
        return EventProcessing::eStop;
    default:
        return EventProcessing::eContinue;
    }
}

EventProcessing ComboBox::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    const math::vec2 pos = position() + offsetByAnchor();
    const math::vec2 s = size();
    setFocused( testRect( p, pos, s ) );
    if ( !isFocused() ) {
        return EventProcessing::eContinue;
    }
    if ( event.type == MouseEvent::eClick ) {
        open();
    }

    return EventProcessing::eStop;
}

void ComboBox::render( const RenderContext& r ) const
{
    auto rctx = r;
    rctx.colorMain = isFocused() ? rctx.colorFocus : rctx.colorMain;
    NineSlice::render( rctx );
}

ComboBoxList::ComboBoxList( const ComboBoxList::CreateInfo& ci ) noexcept
: Widget{}
, m_model{ ci.model }
{
    assert( ci.model );
    setPosition( ci.position + ci.size * math::vec2{ 0.0f, 1.0f } );

    float fSize = static_cast<float>( g_uiProperty.font( "small"_hash )->height() );
    m_lineHeight = fSize + 4.0f;
    m_topPadding = fSize * 0.5f;
    m_index = ScrollIndex{ m_model->current(), m_model->size() };
    auto count = visibleCount();
    float midHeight = m_topPadding + m_lineHeight * static_cast<float>( count );
    m_hover = g_uiProperty.sprite( "mid"_hash );
    setSize( math::vec2{ ci.size.x, midHeight + m_botHeight } );
}

void ComboBoxList::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eSpriteSequenceColors>;
    const float width = size().x;
    RenderInfo ri{
        .m_pipeline = g_uiProperty.pipelineSpriteSequenceColors(),
        .m_verticeCount = PushConstant::VERTICES,
        .m_instanceCount = 1u,
    };
    ri.m_fragmentTexture[ 0 ] = m_hover.texture;
    PushConstant pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    auto count = visibleCount();

    NineSlice::CreateInfo bgci{ .position{ 4.0f, 0.0f }, .size = size(), };
    bgci.size.x -= 8.0f;
    NineSlice bg{ bgci };
    bg.onRender( rctx );

    pushConstant.m_sprites[ 0 ] = {
        .m_color = rctx.colorFocus,
        .m_xywh{ 12.0f, m_topPadding + m_lineHeight * static_cast<float>( m_index.currentVisible() ), width - 24.0f, m_lineHeight },
        .m_uvwh = m_hover,
    };
    ri.m_uniform = pushConstant;
    rctx.renderer->render( ri );

    Label::CreateInfo ci{
        .font = "small"_hash,
        .position{ width - 16.0f, m_topPadding },
        .anchor = Anchor::fRight | Anchor::fTop,
    };

    for ( decltype( count ) i = 0; i < count; ++i ) {
        Label l{ ci };
        l.setText( m_model->at( i + m_index.offset() ) );
        l.onRender( rctx );
        ci.position.y += m_lineHeight;
    }
}

static auto pointToIndex( math::vec2 p, math::vec2 xy, float width, float lineHeight, auto count ) -> decltype( count )
{
    using IDX = decltype( count );
    math::vec2 pos = p - xy;
    if ( pos.x < 0.0f || pos.y < 0.0f ) { return 0; }
    if ( pos.x >= width ) { return 0; }

    IDX idx = static_cast<IDX>( pos.y / lineHeight );
    return ( idx >= count ) ? 0 : ( idx + 1 );
}

EventProcessing ComboBoxList::onMouseEvent( const MouseEvent& event )
{
    const math::vec2 p = event.position;
    math::vec2 pos = position() + offsetByAnchor();
    math::vec2 s = size();
    const bool hitTest = testRect( p, pos, s );
    if ( !hitTest ){
        switch ( event.type ) {
        default: assert( !"unhandled mouse event" );
        case MouseEvent::eClick: return EventProcessing::eStop;
        case MouseEvent::eMove: return EventProcessing::eContinue;
        }
    }

    pos.y += m_topPadding;
    auto idx = pointToIndex( p, pos, s.x, m_lineHeight, visibleCount() );
    if ( idx ) {
        m_index.selectVisible( idx - 1 );
    }

    switch ( event.type ) {
    case MouseEvent::eClick:
        if ( idx ) m_model->select( m_index.current() );
        return EventProcessing::eStop;

    case MouseEvent::eMove:
        return EventProcessing::eContinue;

    default:
        assert( !"unhandled mouse event" );
        return EventProcessing::eStop;
    }

}

EventProcessing ComboBoxList::onAction( ui::Action action )
{
    if ( action.value == 0 ) { return EventProcessing::eContinue; }
    switch ( action.a ) {
    case ui::Action::eMenuConfirm:
        m_model->select( m_index.current() );
        [[fallthrough]];
    case ui::Action::eMenuCancel:
        return EventProcessing::eStop;

    case ui::Action::eMenuDown:
        m_index.increase();
        return EventProcessing::eContinue;

    case ui::Action::eMenuUp:
        m_index.decrease();
        return EventProcessing::eContinue;

    default:
        return EventProcessing::eContinue;
    }
}


DataModel::size_type ComboBoxList::visibleCount() const
{
    return std::min( m_model->size(), m_index.maxVisible() );
}

}
