#pragma once

#include <chrono>
#include <cstdint>

struct FpsLimiter {
    using Clock = std::chrono::high_resolution_clock;

    enum class Mode : uint32_t {
        eOff,
        eSleep,
        eSpinLock,
    };
    using enum Mode;
};
