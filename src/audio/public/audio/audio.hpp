#pragma once

#include <string_view>
#include <cstdint>

class Engine;

class Audio {
protected:
    friend Engine;
    static Audio* create();

public:
    using Slot = uint16_t;

    virtual ~Audio() = default;
    Audio() = default;

    [[nodiscard]]
    virtual Slot load( std::string_view ) = 0;
    virtual void play( Slot ) = 0;
};
