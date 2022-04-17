#include "ui_screen.hpp"

#include "button.hpp"
#include "colors.hpp"
#include "game_action.hpp"
#include "game_callbacks.hpp"
#include "label.hpp"
#include "nineslice.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"
#include "ui_spinbox.hpp"
#include "ui_image.hpp"

#include <unordered_map>
#include <string_view>
using std::literals::string_view_literals::operator""sv;

// TODO load from file
static const std::unordered_map<std::string_view, std::u32string_view> g_locMap{
      { "$loc:gamename", ui::loc::title }
    , { "$loc:newgame", ui::loc::missionSelect }
    , { "$loc:customize", ui::loc::customize }
    , { "$loc:settings", ui::loc::settings }
    , { "$loc:quit", ui::loc::quit }
    , { "$loc:vsync", ui::loc::vsync }
    , { "$loc:return", ui::loc::return2 }
    , { "$loc:resolution", U"RESOLUTION" }
    , { "$loc:weaponPrimary", U"Primary weapon" }
    , { "$loc:weaponSecondary", U"Secondary weapon" }
    , { "$loc:jet", U"Frame" }
    , { "$loc:mission", U"Mission" }
    , { "$loc:missionSelect", U"Mission Select" }
    , { "$loc:engage", U"Engage" }
};

static std::pmr::u32string locKeyToString( std::string_view strv )
{
    auto it = g_locMap.find( strv );
    assert( it != g_locMap.end() );
    return it == g_locMap.end() ? std::pmr::u32string{ strv.begin(), strv.end() } : std::pmr::u32string{ it->second.begin(), it->second.end() };
}

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

static Widget* makeLabel( const cfg::Entry& entry )
{
    Label* label = new Label( g_uiProperty.fontMedium(), Anchor::fTop | Anchor::fLeft, color::white );
    math::vec2 pos = label->position();
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { label->setText( locKeyToString( it.toString() ) ); }
        else if ( propName == "x"sv ) { pos.x = it.toFloat(); }
        else if ( propName == "y"sv ) { pos.y = it.toFloat(); }
        else { assert( !"unhandled label element" ); }
    }
    label->setPosition( pos );
    return label;
}

static Widget* makeButton( const cfg::Entry& entry, int16_t tabOrder )
{
    Button* button = new Button( [](){} );
    button->setAnchor( Anchor::fTop | Anchor::fLeft );
    button->setTabOrder( tabOrder );

    math::vec2 pos = button->position();
    math::vec2 size = button->size();
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "text"sv ) { button->setText( locKeyToString( it.toString() ) ); }
        else if ( propName == "x"sv ) { pos.x = it.toFloat(); }
        else if ( propName == "y"sv ) { pos.y = it.toFloat(); }
        else if ( propName == "width"sv ) { size.x = it.toFloat(); }
        else if ( propName == "height"sv ) { size.y = it.toFloat(); }
        else if ( propName == "trigger"sv ) { button->setTrigger( fnKeyToFunction( it.toString() ) ); }
        else { assert( !"unhandled button element" ); }
    }
    button->setPosition( pos );
    button->setSize( size );
    return button;
}

namespace ui {

static Image* makeImage( const cfg::Entry& entry )
{
    math::vec2 position{};
    math::vec2 extent{};
    DataModel* model = nullptr;
    for ( const auto& property : entry ) {
        auto propName = *property;
        if ( propName == "x"sv ) { position.x = property.toFloat(); continue; }
        if ( propName == "y"sv ) { position.y = property.toFloat(); continue; }
        if ( propName == "width"sv ) { extent.x = property.toFloat(); continue; }
        if ( propName == "height"sv ) { extent.y = property.toFloat(); continue; }
        if ( propName == "data"sv ) { model = dataKeyToModel( property.toString() ); continue; }
        assert( !"unhandled Image property" );
    }
    return new Image{ position, extent, model };
}

static NineSlice* makeNineSlice( const cfg::Entry& entry )
{
    static constexpr std::array<uint32_t, 9> c_slices = {
        ui::AtlasSprite::eTopLeft,
        ui::AtlasSprite::eTop,
        ui::AtlasSprite::eTopRight,
        ui::AtlasSprite::eLeft,
        ui::AtlasSprite::eMid,
        ui::AtlasSprite::eRight,
        ui::AtlasSprite::eBotLeft2,
        ui::AtlasSprite::eBot,
        ui::AtlasSprite::eBotRight2,
    };
    math::vec2 position{};
    math::vec2 extent{};

    for ( const auto& property : entry ) {
        auto propName = *property;
        if ( propName == "x"sv ) { position.x = property.toFloat(); continue; }
        if ( propName == "y"sv ) { position.y = property.toFloat(); continue; }
        if ( propName == "width"sv ) { extent.x = property.toFloat(); continue; }
        if ( propName == "height"sv ) { extent.y = property.toFloat(); continue; }
        assert( !"unhandled property" );
    }

    return new NineSlice(
        position
        , extent
        , g_uiProperty.colorA()
        , Anchor::fTop | Anchor::fLeft
        , g_uiProperty.atlas()
        , c_slices
        , g_uiProperty.atlasTexture()
    );
}

static Widget* makeSpinBox( const cfg::Entry& entry, int16_t tabOrder )
{
    SpinBox* spinbox = new SpinBox{ '0' };
    spinbox->setAnchor( Anchor::fTop | Anchor::fLeft );
    spinbox->setTabOrder( tabOrder );

    math::vec2 pos = spinbox->position();
    math::vec2 size = spinbox->size();
    for ( const auto& it : entry ) {
        auto propName = *it;
        if ( propName == "x"sv ) { pos.x = it.toFloat(); }
        else if ( propName == "y"sv ) { pos.y = it.toFloat(); }
        else if ( propName == "width"sv ) { size.x = it.toFloat(); }
        else if ( propName == "height"sv ) { size.y = it.toFloat(); }
        else if ( propName == "data"sv ) { spinbox->setModel( dataKeyToModel( it.toString() ) ); }
        else { assert( !"unhandled spinbox element" ); }
    }
    spinbox->setPosition( pos );
    spinbox->setSize( size );
    return spinbox;
}


Screen::Screen( const cfg::Entry& entry ) noexcept
{
    uint16_t tabOrderCount = 0;
    for ( const auto& it : entry ) {
        auto str = *it;
        if ( str == "width"sv ) { m_extent.x = it.toFloat(); }
        else if ( str == "height"sv ) { m_extent.y = it.toFloat(); }
        else if ( str == "Label"sv ) { m_widgets.emplace_back( makeLabel( it ) ); }
        else if ( str == "Button"sv ) { m_widgets.emplace_back( makeButton( it, tabOrderCount++ ) ); }
        else if ( str == "SpinBox"sv ) { m_widgets.emplace_back( makeSpinBox( it, tabOrderCount++ ) ); }
        else if ( str == "NineSlice"sv ) { m_widgets.emplace_back( makeNineSlice( it ) ); }
        else if ( str == "Image"sv ) { m_widgets.emplace_back( makeImage( it ) ); }
        else { assert( !"unhandled ui element" ); }
    }
    m_tabOrder = TabOrder<>{ 0, 0, tabOrderCount };
    if ( tabOrderCount == 0 ) { return; }
    changeFocus( Widget::c_invalidTabOrder, 0 );
}

void Screen::render( const RenderContext& rctx ) const
{
    auto r = rctx;
    r.projection = math::ortho( 0.0f, m_extent.x, 0.0f, m_extent.y, -1.0f, 1.0f );
    for ( const auto& it : m_widgets ) {
        it->render( r );
    }
}

void Screen::update( const UpdateContext& uctx )
{
    for ( const auto& it : m_widgets ) {
        it->update( uctx );
    }
}

void Screen::onMouseEvent( const MouseEvent& e )
{
    MouseEvent event = e;
    event.position /= m_resize;
    event.position *= m_extent;
    m_tabOrder.invalidate();
    for ( const auto& it : m_widgets ) {
        if ( it->onMouseEvent( event ) ) {
            m_tabOrder = it->tabOrder();
            return;
        }
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
    Widget* wgt = findWidgetByTabOrder( from );
    if ( wgt ) { wgt->setFocused( false ); }

    wgt = findWidgetByTabOrder( to );
    if ( wgt ) { wgt->setFocused( true ); }
}

void Screen::onAction( Action a )
{
    if ( !a.digital ) { return; }
    const uint16_t prevIndex = *m_tabOrder;
    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuUp: changeFocus( prevIndex, *--m_tabOrder ); break;
    case GameAction::eMenuDown: changeFocus( prevIndex, *++m_tabOrder ); break;
    default:
        if ( Widget* wgt = findWidgetByTabOrder( *m_tabOrder ) ) {
            wgt->onAction( a );
        }
        break;
    }
}

void Screen::resize( math::vec2 s )
{
    m_resize = s;
}








}
