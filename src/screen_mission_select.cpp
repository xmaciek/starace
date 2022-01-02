#include "screen_mission_select.hpp"

#include "colors.hpp"
#include "utils.hpp"

ScreenMissionSelect::ScreenMissionSelect(
    std::pmr::vector<MissionInfo>&& data,
    Font* fontSmall,
    Font* fontMedium,
    Font* fontLarge,
    Widget* rings,
    Texture glow,
    Texture button,
    std::u32string_view enemyTxt,
    std::u32string_view txtPrev, std::function<void()>&& cbPrev,
    std::u32string_view txtNext, std::function<void()>&& cbNext,
    std::u32string_view txtCancel, std::function<void()>&& cbCancel,
    std::u32string_view txtSelect, std::function<void()>&& cbSelect
) noexcept
: m_info{ std::move( data ) }
, m_preview{ Anchor::fCenter | Anchor::fMiddle }
, m_glow{ glow, color::dodgerBlue }
, m_rings{ rings }
, m_title{ fontLarge, Anchor::fCenter | Anchor::fMiddle, {}, color::white }
, m_enemy{ enemyTxt, fontMedium, Anchor::fRight, {}, color::white }
, m_enemyCount{ fontMedium, Anchor::fLeft, {}, color::white }
, m_prev{ txtPrev, fontSmall, button, std::move( cbPrev ) }
, m_next{ txtNext, fontSmall, button, std::move( cbNext ) }
, m_cancel{ txtCancel, fontSmall, button, std::move( cbCancel ) }
, m_select{ txtSelect, fontSmall, button, std::move( cbSelect ) }
{
    assert( !m_info.empty() );
    assert( rings );
    m_preview.setTexture( m_info[ 0 ].m_preview );
    m_title.setText( m_info[ 0 ].m_title );
    m_enemyCount.setText( intToUTF32( m_info[ 0 ].m_enemyCount ) );
    m_prev.setEnabled( false );
    m_next.setEnabled( m_info.size() > 1 );
    m_prev.setAnchor( Anchor::fLeft | Anchor::fMiddle );
    m_next.setAnchor( Anchor::fRight | Anchor::fMiddle );
    m_cancel.setAnchor( Anchor::fRight | Anchor::fMiddle );
    m_select.setAnchor( Anchor::fLeft | Anchor::fMiddle );
    m_prev.setTabOrder( 0 );
    m_cancel.setTabOrder( 1 );
    m_select.setTabOrder( 2 );
    m_next.setTabOrder( 3 );
    updateTabOrderFocus( 2, true );
}

void ScreenMissionSelect::render( RenderContext rctx ) const
{
    m_preview.render( rctx );
    assert( m_rings );
    m_rings->render( rctx );
    m_glow.render( rctx );
    m_title.render( rctx );
    m_enemy.render( rctx );
    m_enemyCount.render( rctx );
    m_prev.render( rctx );
    m_next.render( rctx );
    m_select.render( rctx );
    m_cancel.render( rctx );
}

void ScreenMissionSelect::update( const UpdateContext& uctx )
{
    assert( m_rings );
    m_rings->update( uctx );
}

void ScreenMissionSelect::updateTabOrderFocus( uint32_t idx, bool b )
{
    switch ( idx ) {
    case 0: m_prev.setFocused( b ); break;
    case 1: m_cancel.setFocused( b ); break;
    case 2: m_select.setFocused( b ); break;
    case 3: m_next.setFocused( b ); break;
    }
}

void ScreenMissionSelect::onAction( Action a )
{
    if ( !a.digital ) { return; }
    switch ( a.toA<GameAction>() ) {
    default:
        break;
    case GameAction::eMenuConfirm:
        switch ( m_currentTab ) {
        case 0: m_prev.trigger(); break;
        case 1: m_cancel.trigger(); break;
        case 2: m_select.trigger(); break;
        case 3: m_next.trigger(); break;
        default: break;
        }
        return;
    case GameAction::eMenuCancel:
        m_cancel.trigger();
        break;
    case GameAction::eMenuLeft:
        if ( m_currentTab != c_invalidTabOrder ) {
            updateTabOrderFocus( m_currentTab, false );
        } else {
            m_currentTab = 0;
        }
        if ( m_currentTab == 0 ) { m_currentTab = 3; }
        else { m_currentTab--; }
        updateTabOrderFocus( m_currentTab, true );
        break;

    case GameAction::eMenuRight:
        if ( m_currentTab == c_invalidTabOrder ) {
            m_currentTab = 2;
        }
        else {
            updateTabOrderFocus( m_currentTab, false );
            m_currentTab++;
        }
        if ( m_currentTab > 3 ) { m_currentTab = 0; }
        updateTabOrderFocus( m_currentTab, true );
        break;
    }
}

void ScreenMissionSelect::resize( glm::vec2 wh )
{
    const float max = std::max( wh.x, wh.y );
    m_preview.setSize( glm::vec2{ max, max } );
    m_preview.setPosition( wh * glm::vec2{ 0.5f, 0.5f } );
    m_glow.setSize( wh );
    assert( m_rings );
    m_rings->setSize( wh );
    m_title.setPosition( wh * glm::vec2{ 0.5f, 0.1f } );
    m_enemy.setPosition( wh * glm::vec2{ 0.495f, 0.2f } );
    m_enemyCount.setPosition( wh * glm::vec2{ 0.505f, 0.2f } );
    m_prev.setPosition( wh * glm::vec2{ 0.05f, 0.5f } );
    m_next.setPosition( wh * glm::vec2{ 0.95f, 0.5f } );
    m_cancel.setPosition( wh * glm::vec2{ 0.45f, 0.7f } );
    m_select.setPosition( wh * glm::vec2{ 0.55f, 0.7f } );
}

uint32_t ScreenMissionSelect::selectedMission() const
{
    return m_currentMission;
}

void ScreenMissionSelect::updatePreview()
{
    assert( m_currentMission < m_info.size() );
    const auto& ci = m_info[ m_currentMission ];
    m_preview.setTexture( ci.m_preview );
    m_title.setText( ci.m_title );
    m_enemyCount.setText( intToUTF32( ci.m_enemyCount ) );
    m_prev.setEnabled( m_currentMission != 0 );
    m_next.setEnabled( ( m_currentMission + 1 ) != m_info.size() );
}

bool ScreenMissionSelect::next()
{
    if ( ( m_currentMission + 1 ) == m_info.size() ) {
        return false;
    }
    m_currentMission++;
    updatePreview();
    return true;
}

bool ScreenMissionSelect::prev()
{
    if ( m_currentMission == 0 ) {
        return false;
    }
    m_currentMission--;
    updatePreview();
    return true;
}

bool ScreenMissionSelect::onMouseEvent( const MouseEvent& event )
{
    Widget* wgts[]{
        &m_prev,
        &m_cancel,
        &m_select,
        &m_next,
    };
    for ( auto it : wgts ) {
        if ( !it->onMouseEvent( event ) ) { continue; }
        updateTabOrderFocus( m_currentTab, false );
        m_currentTab = it->tabOrder();
        updateTabOrderFocus( m_currentTab, true );
        return true;
    }
    updateTabOrderFocus( m_currentTab, false );
    m_currentTab = c_invalidTabOrder;
    return {};
}
