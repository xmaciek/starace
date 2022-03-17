#include "screen_mission_select.hpp"

#include "colors.hpp"
#include "ui_localize.hpp"
#include "ui_property.hpp"
#include "utils.hpp"

ScreenMissionSelect::ScreenMissionSelect(
    std::span<const MapCreateInfo> data
    , Widget* rings
    , std::function<void()>&& onPrev
    , std::function<void()>&& onNext
    , std::function<void()>&& onCancel
    , std::function<void()>&& onSelect
) noexcept
: m_info{ data }
, m_preview{ Anchor::fCenter | Anchor::fMiddle }
, m_glow{ color::dodgerBlue }
, m_rings{ rings }
, m_title{ g_uiProperty.fontLarge(), Anchor::fCenter | Anchor::fMiddle, {}, color::white }
, m_enemy{ ui::loc::enemyCount, g_uiProperty.fontMedium(), Anchor::fRight, {}, color::white }
, m_enemyCount{ g_uiProperty.fontMedium(), Anchor::fLeft, {}, color::white }
, m_prev{ ui::loc::missionPrev, Anchor::fLeft | Anchor::fMiddle, std::move( onPrev ) }
, m_next{ ui::loc::missionNext, Anchor::fRight | Anchor::fMiddle, std::move( onNext ) }
, m_cancel{ ui::loc::return2, Anchor::fRight | Anchor::fMiddle, std::move( onCancel ) }
, m_select{ ui::loc::missionStart, Anchor::fLeft | Anchor::fMiddle, std::move( onSelect ) }
, m_currentMission{ 0, 0, static_cast<uint16_t>( m_info.size() ) }
{
    assert( !m_info.empty() );
    assert( rings );
    updatePreview();
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
        if ( !m_currentWidget ) {
            break;
        }
        switch ( *m_currentWidget ) {
        case 0: m_prev.trigger(); break;
        case 1: m_cancel.trigger(); break;
        case 2: m_select.trigger(); break;
        case 3: m_next.trigger(); break;
        default: break;
        }
        break;

    case GameAction::eMenuCancel:
        m_cancel.trigger();
        break;

    case GameAction::eMenuLeft:
        if ( m_currentWidget ) { updateTabOrderFocus( *m_currentWidget, false ); }
        m_currentWidget--;
        updateTabOrderFocus( *m_currentWidget, true );
        break;

    case GameAction::eMenuRight:
        if ( m_currentWidget ) { updateTabOrderFocus( *m_currentWidget, false ); }
        m_currentWidget++;
        updateTabOrderFocus( *m_currentWidget, true );
        break;
    }
}

void ScreenMissionSelect::resize( math::vec2 wh )
{
    const float max = std::max( wh.x, wh.y );
    m_preview.setSize( math::vec2{ max, max } );
    m_preview.setPosition( wh * math::vec2{ 0.5f, 0.5f } );
    m_glow.setSize( wh );
    assert( m_rings );
    m_rings->setSize( wh );
    m_title.setPosition( wh * math::vec2{ 0.5f, 0.1f } );
    m_enemy.setPosition( wh * math::vec2{ 0.495f, 0.2f } );
    m_enemyCount.setPosition( wh * math::vec2{ 0.505f, 0.2f } );
    m_prev.setPosition( wh * math::vec2{ 0.05f, 0.5f } );
    m_next.setPosition( wh * math::vec2{ 0.95f, 0.5f } );
    m_cancel.setPosition( wh * math::vec2{ 0.45f, 0.7f } );
    m_select.setPosition( wh * math::vec2{ 0.55f, 0.7f } );
}

uint32_t ScreenMissionSelect::selectedMission() const
{
    return *m_currentMission;
}

void ScreenMissionSelect::updatePreview()
{
    assert( m_currentMission );
    const auto& ci = m_info[ *m_currentMission ];
    m_preview.setTexture( ci.preview );
    m_title.setText( ci.name );
    m_enemyCount.setText( intToUTF32( ci.enemies ) );
}

bool ScreenMissionSelect::next()
{
    m_currentMission++;
    updatePreview();
    return true;
}

bool ScreenMissionSelect::prev()
{
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
        if ( m_currentWidget ) {
            updateTabOrderFocus( *m_currentWidget, false );
        }
        const auto idx = it->tabOrder();
        m_currentWidget = idx;
        updateTabOrderFocus( idx, true );
        return true;
    }
    if ( m_currentWidget ) {
        updateTabOrderFocus( *m_currentWidget, false );
    }
    m_currentWidget.invalidate();
    return {};
}
