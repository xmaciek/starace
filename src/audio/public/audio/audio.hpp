#pragma once

#include <cstdint>
#include <span>

class Engine;

class Audio {
protected:
    friend Engine;
    static Audio* create();

public:
    using Slot = uint16_t;
    static constexpr uint16_t c_invalidSlot = 0xFFFFu;

    virtual ~Audio() = default;
    Audio() = default;

    [[nodiscard]]
    virtual Slot load( std::span<const uint8_t> ) = 0;
    virtual void play( Slot ) = 0;
};
