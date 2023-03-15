#include "hud.hpp"

#include "colors.hpp"
#include "utils.hpp"
#include <ui/property.hpp>

Hud::Hud( const HudData* displayData ) noexcept
: m_displayData{ displayData }
, m_score{ Label::CreateInfo{ .text = U"Score: ", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_scoreValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_shots{ Label::CreateInfo{ .text = U"Shots: ", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_shotsValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_pool{ Label::CreateInfo{ .text = U"Pool: ", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_poolValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_fps{ Label::CreateInfo{ .text = U"FPS: ", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_fpsValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_calc{ Label::CreateInfo{ .text = U"Calculated: ", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_calcValue{ Label::CreateInfo{ .text = U"0", .font = g_uiProperty.fontSmall(), .color = color::winScreen } }
, m_speedMeter{ nullptr }
, m_hp{ U"HP" }
{
    Widget* arr[] = {
        &m_score,
        &m_shots,
        &m_pool,
        &m_fps,
        &m_calc,
    };
    Layout{ { 4.0f, 4.0f }, Layout::eVertical }( arr );

    const auto rightOf = []( const Label& w ) {
        return w.position() + math::vec2{ w.size().x, 0.0f };
    };

    m_scoreValue.setPosition( rightOf( m_score ) );
    m_shotsValue.setPosition( rightOf( m_shots ) );
    m_poolValue.setPosition( rightOf( m_pool ) );
    m_fpsValue.setPosition( rightOf( m_fps ) );
    m_calcValue.setPosition( rightOf( m_calc ) );
}

void Hud::render( ui::RenderContext rctx ) const
{
    const std::array arr = {
        &m_score,
        &m_scoreValue,
        &m_shots,
        &m_shotsValue,
        &m_pool,
        &m_poolValue,
        &m_fps,
        &m_fpsValue,
        &m_calc,
        &m_calcValue,
    };
    for ( const auto* it : arr ) {
        it->render( rctx );
    }
    m_speedMeter.render( rctx );
    m_hp.render( rctx );
}

void Hud::update( const UpdateContext& uctx )
{
    constexpr auto setIf = []( uint32_t a, uint32_t b, Label* lbl )
    {
        if ( a != b ) {
            lbl->setText( intToUTF32( a ) );
        }
    };
    setIf( m_displayData->score, m_lastData.score, &m_scoreValue );
    setIf( m_displayData->shots, m_lastData.shots, &m_shotsValue );
    setIf( m_displayData->pool, m_lastData.pool, &m_poolValue );
    setIf( m_displayData->fps, m_lastData.fps, &m_fpsValue );
    setIf( m_displayData->calc, m_lastData.calc, &m_calcValue );
    m_speedMeter.setSpeed( m_displayData->speed );
    m_speedMeter.update( uctx );
    m_hp.setValue( m_displayData->hp );

    m_lastData = *m_displayData;
}

void Hud::resize( math::vec2 s )
{
    m_speedMeter.setPosition( math::vec2{ 32.0f, 0.0f } + s * math::vec2{ 0.0f, 0.5f } );

    Widget* bars[] = {
        &m_hp,
    };
    Layout{ { 4.0f, s.y - 4.0f }, Layout::eHorizontal }( bars );
}
