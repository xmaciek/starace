#pragma once

#include <chrono>

class FPSMeter {
    using clock_type = std::chrono::steady_clock;
    clock_type::duration m_accumulatedDuration{};
    clock_type::time_point m_lastMeasure{};
    clock_type::time_point m_lastPoint{};
    uint32_t m_framesDone = 0;
    uint32_t m_framesReallyDone = 0;
    float m_lastFpsValue = 0.0f;
public:
    FPSMeter();
    void frameBegin();
    void frameEnd();
    uint32_t fps() const;
    float calculated() const;
};
