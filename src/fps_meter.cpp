#include "fps_meter.hpp"

#include <cassert>
#include <utility>

FPSMeter::FPSMeter()
{
    m_lastPoint = clock_type::now();
    m_lastMeasure = clock_type::now();
}

void FPSMeter::frameEnd()
{
    const clock_type::time_point tp = clock_type::now();
    m_accumulatedDuration += tp - m_lastMeasure;
    m_framesDone++;

    const uint32_t seconds = std::chrono::duration_cast<std::chrono::seconds>( tp - m_lastPoint ).count();
    if ( seconds == 0 ) {
        return;
    }
    m_lastPoint = tp;
    uint32_t framesDone = 0;
    std::swap( framesDone, m_framesDone );
    m_framesReallyDone = framesDone;
    const uint64_t usec = std::chrono::duration_cast<std::chrono::microseconds>( m_accumulatedDuration ).count();
    m_accumulatedDuration = std::chrono::seconds( 0 );
    const float averageFrameDuration = static_cast<float>( usec ) / framesDone;
    m_lastFpsValue = 1'000'000.0f / averageFrameDuration;
}

void FPSMeter::frameBegin()
{
    m_lastMeasure = clock_type::now();
}

float FPSMeter::calculated() const
{
    return m_lastFpsValue;
}

uint32_t FPSMeter::fps() const
{
    return m_framesReallyDone;
}
