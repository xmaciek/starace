#include "hud.hpp"

#include "colors.hpp"
#include "utils.hpp"

Hud::Hud( const HudData* displayData, Font* font ) noexcept
: m_displayData{ displayData }
, m_score{ U"Score: ", font, {}, color::winScreen }
, m_scoreValue{ U"", font, {}, color::winScreen }
, m_shots{ U"Shots: ", font, {}, color::winScreen }
, m_shotsValue{ U"", font, {}, color::winScreen }
, m_pool{ U"Pool: ", font, {}, color::winScreen }
, m_poolValue{ U"", font, {}, color::winScreen }
, m_fps{ U"FPS: ", font, {}, color::winScreen }
, m_fpsValue{ U"", font, {}, color::winScreen }
, m_calc{ U"Calculated: ", font, {}, color::winScreen }
, m_calcValue{ U"", font, {}, color::winScreen }
, m_speedMeter{ font }
, m_hp{ U"HP", font }
, m_pwr{ U"PWR", font }
{
    Widget* arr[] = {
        &m_score,
        &m_shots,
        &m_pool,
        &m_fps,
        &m_calc,
    };
    Layout{ { 4.0f, 4.0f }, Layout::eVertical }( std::begin( arr ), std::end( arr ) );

    const auto rightOf = []( const Label& w ) {
        return w.position() + glm::vec2{ w.size().x, 0.0f };
    };

    m_scoreValue.setPosition( rightOf( m_score ) );
    m_shotsValue.setPosition( rightOf( m_shots ) );
    m_poolValue.setPosition( rightOf( m_pool ) );
    m_fpsValue.setPosition( rightOf( m_fps ) );
    m_calcValue.setPosition( rightOf( m_calc ) );
}

void Hud::render( RenderContext rctx ) const
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
    m_pwr.render( rctx );
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
    m_pwr.setValue( m_displayData->pwr );

    m_lastData = *m_displayData;
}

void Hud::resize( glm::vec2 s )
{
    m_speedMeter.setPosition( glm::vec2{ 32.0f, 0.0f } + s * glm::vec2{ 0.0f, 0.5f } );

    Widget* bars[] = {
        &m_hp,
        &m_pwr,
    };
    Layout{ { 4.0f, s.y - 4.0f }, Layout::eHorizontal }( std::begin( bars ), std::end( bars ) );
}
