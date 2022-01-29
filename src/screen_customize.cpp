#include "screen_customize.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "game_action.hpp"

#include <cassert>

using std::literals::string_view_literals::operator""sv;

constexpr std::array c_weapName = { U"Laser"sv, U"Blaster"sv, U"Torpedo"sv };

ScreenCustomize::ScreenCustomize(
    std::array<uint32_t, 3> equipment
    , std::pmr::vector<CustomizeData>&& jets
    , Font* fontSmall
    , Font* fontMedium
    , Texture btn
    , Widget* rings
    , std::u32string_view done, std::function<void()>&& onDone
    , std::u32string_view pJet, std::function<void()>&& onPrev
    , std::u32string_view nJet, std::function<void()>&& onNext
    , std::function<void()>&& w1
    , std::function<void()>&& w2
    , std::function<void()>&& w3
) noexcept
: m_jets{ std::move( jets ) }
, m_currentRotation{ 135.0_deg }
, m_rings{ rings }
, m_weap1{ equipment[ 0 ] }
, m_weap2{ equipment[ 1 ] }
, m_weap3{ equipment[ 2 ] }
, m_glow{ color::dodgerBlue }
, m_jetName{ fontMedium, Anchor::fCenter | Anchor::fMiddle, color::white }
, m_jetPrev{ pJet, fontSmall, btn, Anchor::fLeft | Anchor::fMiddle, std::move( onPrev ) }
, m_jetNext{ nJet, fontSmall, btn, Anchor::fRight | Anchor::fMiddle, std::move( onNext ) }
, m_done{ done, fontSmall, btn, Anchor::fCenter | Anchor::fMiddle, std::move( onDone ) }
, m_btnWeap1{ c_weapName[ equipment[ 0 ] ], fontSmall, btn, Anchor::fRight | Anchor::fMiddle, std::move( w1 ) }
, m_btnWeap2{ c_weapName[ equipment[ 1 ] ], fontSmall, btn, Anchor::fCenter | Anchor::fMiddle, std::move( w2 ) }
, m_btnWeap3{ c_weapName[ equipment[ 2 ] ], fontSmall, btn, Anchor::fLeft | Anchor::fMiddle, std::move( w3 ) }
{
    assert( m_rings );
    assert( !m_jets.empty() );
    m_jetPrev.setTabOrder( 0 );
    m_btnWeap1.setTabOrder( 1 );
    m_btnWeap2.setTabOrder( 2 );
    m_btnWeap3.setTabOrder( 3 );
    m_jetNext.setTabOrder( 4 );
    updateInfo();
}

void ScreenCustomize::render( RenderContext rctx ) const
{
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );
    {
        RenderContext rctx2 = rctx;
        rctx2.projection = math::perspective(
            55.0_deg
            , m_size.x / m_size.y
            , 0.001f
            , 2000.0f
        );
        const float s = m_jets[ m_currentJet ].m_jetUiScale;
        rctx2.model = math::translate( rctx2.model, math::vec3{ 0, 0.1, -1.25 } );
        rctx2.model = math::scale( rctx2.model, math::vec3{ s, s, s } );
        rctx2.model = math::rotate( rctx2.model, -15.0_deg, axis::x );
        rctx2.model = math::rotate( rctx2.model, m_currentRotation, axis::y );
        assert( m_jets[ m_currentJet ].m_jetModel );
        m_jets[ m_currentJet ].m_jetModel->render( rctx2 );
    }
    m_jetName.render( rctx );
    m_jetPrev.render( rctx );
    m_jetNext.render( rctx );
    m_btnWeap1.render( rctx );
    m_btnWeap2.render( rctx );
    m_btnWeap3.render( rctx );
    m_done.render( rctx );
}

void ScreenCustomize::update( const UpdateContext& uctx )
{
    assert( m_rings );
    m_rings->update( uctx );
    m_currentRotation += 30.0_deg * uctx.deltaTime;
}

bool ScreenCustomize::onMouseEvent( const MouseEvent& event )
{
    uint32_t o = c_invalidTabOrder;
    if ( m_jetPrev.onMouseEvent( event ) ) { o = m_jetPrev.tabOrder(); }
    else if ( m_jetNext.onMouseEvent( event ) ) { o = m_jetNext.tabOrder(); }
    else if ( m_btnWeap1.onMouseEvent( event ) ) { o = m_btnWeap1.tabOrder(); }
    else if ( m_btnWeap2.onMouseEvent( event ) ) { o = m_btnWeap2.tabOrder(); }
    else if ( m_btnWeap3.onMouseEvent( event ) ) { o = m_btnWeap3.tabOrder(); }
    else {
        m_currentTabOrder = c_invalidTabOrder;
        return m_done.onMouseEvent( event );
    }

    m_currentTabOrder = o;
    updateFocus();
    return true;
}

void ScreenCustomize::updateFocus()
{
    Button* btns[] = {
        &m_jetPrev,
        &m_btnWeap1,
        &m_btnWeap2,
        &m_btnWeap3,
        &m_jetNext,
    };
    for ( auto* it : btns ) {
        it->setFocused( it->tabOrder() == m_currentTabOrder );
    }
}

void ScreenCustomize::onAction( Action a )
{
    if ( !a.digital ) { return; }

    switch ( a.toA<GameAction>() ) {
    case GameAction::eMenuCancel:
        m_done.trigger();
        break;

    case GameAction::eMenuConfirm: {
        if ( m_currentTabOrder == c_invalidTabOrder ) {
            if ( m_done.isFocused() ) { m_done.trigger(); }
            break;
        }
        Button* btns[] = {
            &m_jetPrev,
            &m_btnWeap1,
            &m_btnWeap2,
            &m_btnWeap3,
            &m_jetNext,
        };
        for ( auto* it : btns ) {
            if ( it->tabOrder() != m_currentTabOrder ) { continue; }
            it->trigger();
            break;
        }
    } break;

    case GameAction::eMenuLeft:
        switch ( m_currentTabOrder ) {
        case c_invalidTabOrder:
            m_done.setFocused( false );
            m_currentTabOrder = 1;
            break;
        case 0: m_currentTabOrder = 4; break;
        case 1:
        case 2:
        case 3:
        case 4:
            m_currentTabOrder--; break;
        default:
            break;
        }
        updateFocus();
        break;

    case GameAction::eMenuRight:
        switch ( m_currentTabOrder ) {
        case c_invalidTabOrder:
            m_currentTabOrder = 3;
            m_done.setFocused( false );
            break;
        case 4: m_currentTabOrder = 0; break;
        case 0:
        case 1:
        case 2:
        case 3:
            m_currentTabOrder++; break;
        default:
            break;
        }
        updateFocus();
        break;

    case GameAction::eMenuDown:
        m_currentTabOrder = c_invalidTabOrder;
        updateFocus();
        m_done.setFocused( true );
        break;

    case GameAction::eMenuUp:
        if ( m_currentTabOrder != c_invalidTabOrder ) { break; }
        m_done.setFocused( false );
        m_currentTabOrder = 2;
        updateFocus();
        break;
    default:
        break;
    }
}

void ScreenCustomize::resize( math::vec2 wh )
{
    setSize( wh );
    m_rings->setSize( wh );
    m_glow.setSize( wh );
    m_jetName.setPosition( wh * math::vec2{ 0.5f, 0.1f } );
    m_jetPrev.setPosition( wh * math::vec2{ 0.1f, 0.5f } );
    m_jetNext.setPosition( wh * math::vec2{ 0.9f, 0.5f } );
    m_done.setPosition( wh * math::vec2{ 0.5f, 0.9f } );
    m_btnWeap1.setPosition( wh * math::vec2{ 0.4f, 0.8f } );
    m_btnWeap2.setPosition( wh * math::vec2{ 0.5f, 0.8f } );
    m_btnWeap3.setPosition( wh * math::vec2{ 0.6f, 0.8f } );
}

bool ScreenCustomize::nextJet()
{
    if ( m_currentJet + 1 == m_jets.size() ) { return false; }
    m_currentJet += 1;
    updateInfo();
    return true;
}

bool ScreenCustomize::prevJet()
{
    if ( m_currentJet == 0 ) { return false; }
    m_currentJet -= 1;
    updateInfo();
    return true;
}

void ScreenCustomize::nextWeap( uint32_t w )
{
    switch ( w ) {
    case 0: m_btnWeap1.setText( c_weapName[ *++m_weap1 ] ); break;
    case 1: m_btnWeap2.setText( c_weapName[ *++m_weap2 ] ); break;
    case 2: m_btnWeap3.setText( c_weapName[ *++m_weap3 ] ); break;
    }
}

std::array<uint32_t, 3> ScreenCustomize::weapons() const
{
    return { *m_weap1, *m_weap2, *m_weap3 };
}

uint32_t ScreenCustomize::jet() const
{
    return m_currentJet;
}

void ScreenCustomize::updateInfo()
{
    m_jetName.setText( m_jets[ m_currentJet ].m_jetName );
    m_jetPrev.setEnabled( m_currentJet != 0 );
    m_jetNext.setEnabled( m_currentJet + 1 < m_jets.size() );
}
