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

    enum class Channel : uint8_t {
        eMaster,
        eMusic,
        eSFX,
        eUI,
        count,
    };
    virtual ~Audio() = default;
    Audio() = default;

    [[nodiscard]]
    virtual Slot load( std::span<const uint8_t> ) = 0;
    virtual void play( Slot, Channel ) = 0;
    virtual void setVolume( Channel, float ) = 0;
};
