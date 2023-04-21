#include "ui_screen.hpp"

#include "colors.hpp"
#include "game_callbacks.hpp"
#include "progressbar.hpp"

#include <ui/button.hpp>
#include <ui/combobox.hpp>
#include <ui/image.hpp>
#include <ui/label.hpp>
#include <ui/nineslice.hpp>
#include <ui/property.hpp>
#include <ui/spinbox.hpp>

#include <Tracy.hpp>

#include <unordered_map>
#include <string_view>

using std::literals::string_view_literals::operator""sv;

static auto fnKeyToFunction( std::string_view strv )
{
    auto it = g_gameCallbacks.find( strv );
    assert( it != g_gameCallbacks.end() );
    return it == g_gameCallbacks.end() ? [](){} : it->second;
}

static auto dataKeyToModel( std::string_view strv )
{
    auto it = g_gameUiDataModels.find( strv );
    assert( it != g_gameUiDataModels.end() );
    assert( it->second );
    return it != g_gameUiDataModels.end() ? it->second : nullptr;
}


namespace ui {

static UniquePointer<Widget> makeButton( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    std::pmr::u32string text{};
    Button::CreateInfo ci{
        .tabOrder = tabOrder,
    };

    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "height"_hash: ci.size.y = property.toFloat(); continue;
        case "text"_hash: text = g_uiProperty.localize( property.toString() ); continue;
        case "trigger"_hash: ci.trigger = fnKeyToFunction( property.toString() ); continue;
        case "width"_hash: ci.size.x = property.toFloat(); continue;
        case "x"_hash: ci.position.x = property.toFloat(); continue;
        case "y"_hash: ci.position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled Button element" );
            continue;
        }
    }
    ci.text = text;
    return UniquePointer<Button>{ alloc, ci };
}

static UniquePointer<Widget> makeImage( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    Image::CreateInfo ci{};

    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "data"_hash: ci.model = dataKeyToModel( property.toString() ); continue;
        case "height"_hash: ci.size.y = property.toFloat(); continue;
        case "width"_hash: ci.size.x = property.toFloat(); continue;
        case "x"_hash: ci.position.x = property.toFloat(); continue;
        case "y"_hash: ci.position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled Image property" );
            continue;
        }
    }
    return UniquePointer<Image>{ alloc, ci };
}

static UniquePointer<Widget> makeNineSlice( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    static constexpr std::array<ui::Atlas::hash_type, 9> SLICES = {
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
    math::vec2 position{};
    math::vec2 extent{};

    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "height"_hash: extent.y = property.toFloat(); continue;
        case "width"_hash: extent.x = property.toFloat(); continue;
        case "x"_hash: position.x = property.toFloat(); continue;
        case "y"_hash: position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled NineSlice property" );
            continue;
        }
    }

    return UniquePointer<NineSlice>{ alloc
        , position
        , extent
        , Anchor::fTop | Anchor::fLeft
        , SLICES
    };
}

static UniquePointer<Widget> makeLabel( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    Label::CreateInfo ci{
        .font = g_uiProperty.fontMedium(),
    };
    std::u32string text{};
    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "data"_hash: ci.dataModel = dataKeyToModel( property.toString() ); continue;
        case "text"_hash: text = g_uiProperty.localize( property.toString() ); continue;
        case "x"_hash: ci.position.x = property.toFloat(); continue;
        case "y"_hash: ci.position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled Label property" );
            continue;
        }
    }
    ci.text = text;
    return UniquePointer<Label>{ alloc, ci };
}

static UniquePointer<Widget> makeSpinBox( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    math::vec2 pos{};
    math::vec2 size{};
    DataModel* dataModel = nullptr;
    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( *property ) ) {
        case "data"_hash: dataModel = dataKeyToModel( property.toString() ); continue;
        case "height"_hash: size.y = property.toFloat(); continue;
        case "width"_hash: size.x = property.toFloat(); continue;
        case "x"_hash: pos.x = property.toFloat(); continue;
        case "y"_hash: pos.y = property.toFloat(); continue;
        default:
            assert( !"unhandled SpinBox element" );
            continue;
        }
    }

    assert( dataModel );
    UniquePointer<SpinBox> spinbox{ alloc, dataModel };
    spinbox->setTabOrder( tabOrder );
    spinbox->setPosition( pos );
    spinbox->setSize( size );
    return spinbox;
}


static UniquePointer<Widget> makeComboBox( std::pmr::memory_resource* alloc, const cfg::Entry& entry, uint16_t tabOrder )
{
    assert( alloc );
    ComboBox::CreateInfo ci{};
    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( auto h = hash( *property ) ) {
        case "data"_hash: ci.model = dataKeyToModel( property.toString() ); continue;
        case "height"_hash: ci.size.y = property.toFloat(); continue;
        case "text"_hash: ci.text = g_uiProperty.localize( property.toString() ); continue;
        case "width"_hash: ci.size.x = property.toFloat(); continue;
        case "x"_hash: ci.position.x = property.toFloat(); continue;
        case "y"_hash: ci.position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled Entry element" );
            continue;
        }
    }

    assert( ci.model );
    UniquePointer<ComboBox> ptr{ alloc, ci };
    ptr->setTabOrder( tabOrder );
    return ptr;
}

static UniquePointer<Widget> makeProgressbar( std::pmr::memory_resource* alloc, const cfg::Entry& entry )
{
    assert( alloc );
    Progressbar::CreateInfo ci{};
    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( auto h = hash( *property ) ) {
        case "data"_hash: ci.model = dataKeyToModel( property.toString() ); continue;
        case "x"_hash: ci.position.x = property.toFloat(); continue;
        case "y"_hash: ci.position.y = property.toFloat(); continue;
        default:
            assert( !"unhandled Entry element" );
            continue;
        }
    }

    assert( ci.model );
    return UniquePointer<Progressbar>{ alloc, ci };
}

Screen::Screen( const cfg::Entry& entry ) noexcept
{
    ZoneScoped;
    std::pmr::memory_resource* alloc = std::pmr::get_default_resource();
    uint16_t tabOrderCount = 0;

    Hash hash{};
    for ( const auto& property : entry ) {
        switch ( hash( * property ) ) {
        case "Button"_hash: m_widgets.emplace_back( makeButton( alloc, property, tabOrderCount++ ) ); continue;
        case "ComboBox"_hash: m_widgets.emplace_back( makeComboBox( alloc, property, tabOrderCount++ ) ); continue;
        case "Image"_hash: m_widgets.emplace_back( makeImage( alloc, property ) ); continue;
        case "Label"_hash: m_widgets.emplace_back( makeLabel( alloc, property ) ); continue;
        case "NineSlice"_hash: m_widgets.emplace_back( makeNineSlice( alloc, property ) ); continue;
        case "Progressbar"_hash: m_widgets.emplace_back( makeProgressbar( alloc, property ) ); continue;
        case "SpinBox"_hash: m_widgets.emplace_back( makeSpinBox( alloc, property, tabOrderCount++ ) ); continue;
        case "height"_hash: m_extent.y = property.toFloat(); continue;
        case "width"_hash: m_extent.x = property.toFloat(); continue;
        default:
            assert( !"unhandled ui element" );
            continue;
        }
    }
    if ( tabOrderCount == 0 ) { return; }
    m_tabOrder = TabOrder<>{ 0, 0, tabOrderCount };
    changeFocus( Widget::c_invalidTabOrder, 0 );
}

void Screen::render( const RenderContext& rctx ) const
{
    ZoneScoped;
    auto r = rctx;
    r.projection = math::ortho( 0.0f, m_viewport.x, 0.0f, m_viewport.y, -1.0f, 1.0f );
    const math::mat4 view = math::translate( r.view, math::vec3{ m_offset.x, m_offset.y, 0.0f } );

    auto startPosistionForWidget = []( const auto& wgt, float viewportX, float extentX )
    {
        const math::vec2 pos = wgt->position() + wgt->offsetByAnchor();
        const float left = pos.x;
        const float right = extentX - ( pos.x + wgt->size().x );
        return ( left < right ) ? -viewportX : viewportX;
    };

    for ( const auto& it : m_widgets ) {
        const float startPos = startPosistionForWidget( it, m_viewport.x, m_extent.x );
        const float animX = math::nonlerp( startPos, 0.0f, m_anim );
        r.view = math::translate( view, math::vec3{ animX, 0.0f, 0.0f } );
        it->render( r );
    }

    if ( m_comboBoxList ) m_comboBoxList->render( r );
}

void Screen::update( const UpdateContext& uctx )
{
    ZoneScoped;
    if ( auto c = g_uiProperty.pendingModalComboBox(); c.model ) {
        g_uiProperty.requestModalComboBox( {}, {}, {} );
        ComboBoxList::CreateInfo ci{ .model = c.model, .position = c.position, .size = c.size };
        m_comboBoxList = UniquePointer<ComboBoxList>( std::pmr::get_default_resource(), ci );
    }
    m_anim = std::clamp( m_anim + uctx.deltaTime * 5.0f, 0.0f, 1.0f );
    for ( const auto& it : m_widgets ) {
        it->update( uctx );
    }
    if ( m_comboBoxList ) m_comboBoxList->update( uctx );
}

void Screen::onMouseEvent( const MouseEvent& e )
{
    MouseEvent event = e;
    event.position /= m_resize;
    event.position *= m_viewport;
    event.position -= m_offset;
    if ( m_comboBoxList ) {
        auto mouseProcessing = m_comboBoxList->onMouseEvent( event );
        if ( mouseProcessing == EventProcessing::eStop ) {
            m_comboBoxList = {};
        }
        return;
    }
    uint16_t prevWidget = *m_tabOrder;
    m_tabOrder.invalidate();
    for ( const auto& it : m_widgets ) {
        auto mouseProcessing = it->onMouseEvent( event );
        if ( mouseProcessing == EventProcessing::eContinue ) continue;
        m_tabOrder = it->tabOrder();
        changeFocus( prevWidget, *m_tabOrder );
        return;
    }

}

Widget* Screen::findWidgetByTabOrder( uint16_t tabOrder )
{
    if ( tabOrder == Widget::c_invalidTabOrder ) {
        return nullptr;
    }

    auto wgt = std::find_if( m_widgets.begin(), m_widgets.end()
        , [tabOrder]( const auto& wgt ) { return wgt->tabOrder() == tabOrder; }
    );
    return ( wgt != m_widgets.end() ) ? wgt->get() : nullptr;
}

void Screen::changeFocus( uint16_t from, uint16_t to )
{
    if ( from == to ) { return; }
    Widget* wgt = findWidgetByTabOrder( from );
    if ( wgt ) { wgt->setFocused( false ); }

    wgt = findWidgetByTabOrder( to );
    if ( wgt ) { wgt->setFocused( true ); }
}

void Screen::onAction( ui::Action action )
{
    ZoneScoped;
    if ( action.value == 0 ) { return; }

    if ( m_comboBoxList ) {
        auto actionProcessing = m_comboBoxList->onAction( action );
        if ( actionProcessing == EventProcessing::eStop ) {
            m_comboBoxList = {};
        }
        return;
    }

    const uint16_t prevIndex = *m_tabOrder;
    switch ( action.a ) {
    case ui::Action::eMenuUp: changeFocus( prevIndex, *--m_tabOrder ); break;
    case ui::Action::eMenuDown: changeFocus( prevIndex, *++m_tabOrder ); break;
    default:
        if ( Widget* wgt = findWidgetByTabOrder( *m_tabOrder ) ) {
            wgt->onAction( action );
        }
        break;
    }
}

void Screen::resize( math::vec2 s )
{
    m_resize = s;
    const float aspect = m_resize.x / m_resize.y;
    const float srcAspect = m_extent.x / m_extent.y;
    m_viewport = ( aspect >= srcAspect )
        ? math::vec2{ m_extent.y * aspect, m_extent.y }
        : math::vec2{ m_extent.x, m_extent.x * ( m_resize.y / m_resize.x ) };
    m_offset = ( m_viewport - m_extent ) * 0.5f;
}

void Screen::show( math::vec2 size )
{
    resize( size );
    m_anim = 0.0f;
}





}
