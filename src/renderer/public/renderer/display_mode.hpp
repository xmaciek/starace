#pragma once

#include <cstdint>

struct DisplayMode {
    uint16_t width;
    uint16_t height;
    uint16_t monitor;
    bool fullscreen;
};

enum class VSync : uint32_t {
    eOff,
    eOn,
    eMailbox,
};

