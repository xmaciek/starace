#pragma once

#include <string_view>
#include <cstdint>

class Engine;

class Audio {
    friend Engine;
    static Audio* create();

protected:
    Audio() = default;

public:
    using Slot = uint16_t;

    virtual ~Audio() = default;

    virtual void play( Slot ) = 0;
    virtual Slot load( std::string_view ) = 0;
};
