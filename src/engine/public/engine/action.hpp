#pragma once

#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>

#include <compare>
#include <cstdint>
#include <limits>

struct Actuator {
    // TODO: use custom unified enum, while for now i'll keep these
    static_assert( sizeof( SDL_Scancode ) == 4 );
    static_assert( sizeof( SDL_GameControllerButton ) == 4 );
    static_assert( sizeof( SDL_GameControllerAxis ) == 4 );
    using Scancode = SDL_Scancode;
    using Buttoncode = SDL_GameControllerButton;
    using Axiscode = SDL_GameControllerAxis;

    static constexpr inline uint16_t SCANCODE = 1;
    static constexpr inline uint16_t BUTTONCODE = 2;
    static constexpr inline uint16_t AXISCODE = 3;

    using Limits = std::numeric_limits<int16_t>;
    static constexpr inline int16_t MAX = Limits::max();
    static constexpr inline int16_t MIN = Limits::min();
    static constexpr inline int16_t NOMINAL = 0;

    struct Typed {
        uint16_t type : 2;
        uint16_t id : 14;
    };

    union {
        uint16_t raw = 0;
        Typed typed;
    };
    int16_t value = 0;

    constexpr Actuator() noexcept = default;
    constexpr Actuator( Scancode s, bool v = false ) noexcept
    : typed{ .type = SCANCODE, .id = (uint16_t)s }, value{ v ? MAX : NOMINAL } {};

    constexpr Actuator( Buttoncode b, bool v = false ) noexcept
    : typed{ .type = BUTTONCODE, .id = (uint16_t)b }, value{ v ? MAX : NOMINAL } {};

    constexpr Actuator( Axiscode a, int16_t v = 0 ) noexcept
    : typed{ .type = AXISCODE, .id = (uint16_t)a }, value{ v } {};

    constexpr std::strong_ordering operator <=> ( const Actuator& rhs ) const noexcept
    {
        return raw <=> rhs.raw;
    }
};
static_assert( sizeof( Actuator ) == 4 );
static_assert( alignof( Actuator ) == 2 );

struct Action {
    using Enum = uint16_t;
    union {
        float analog = 0.0f;
        bool digital;
    };
    Enum userEnum{};
    template  <typename T>
    T toA() const noexcept
    {
        return static_cast<T>( userEnum );
    }
};
static_assert( sizeof( Action ) == 8 );
static_assert( alignof( Action ) == 4 );
