#pragma once

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_scancode.h>

#include <compare>
#include <cstdint>

using UserEnumUType = uint16_t;

struct Actuator {
    // TODO: use custom unified enum, while for now i'll keep these
    static_assert( sizeof( SDL_Scancode ) == 4 );
    static_assert( sizeof( SDL_GameControllerButton ) == 4 );
    static_assert( sizeof( SDL_GameControllerAxis ) == 4 );
    using Scancode = SDL_Scancode;
    using Buttoncode = SDL_GameControllerButton;
    using Axiscode = SDL_GameControllerAxis;

    enum class Type : uint32_t {
        eNone,
        eScancode,
        eButtoncode,
        eAxiscode,
    };
    union {
        Scancode scancode{};
        Buttoncode buttoncode;
        Axiscode axiscode;
    };
    Type type{};

    constexpr Actuator() noexcept = default;
    constexpr Actuator( Scancode s ) noexcept : scancode{ s }, type{ Type::eScancode } {};
    constexpr Actuator( Buttoncode s ) noexcept : buttoncode{ s }, type{ Type::eButtoncode } {};
    constexpr Actuator( Axiscode s ) noexcept : axiscode{ s }, type{ Type::eAxiscode } {};

    constexpr std::strong_ordering operator <=> ( const Actuator& rhs ) const noexcept
    {
        if ( type != rhs.type ) { return type <=> rhs.type; }
        switch ( type ) {
        default: return std::strong_ordering::equivalent;
        case Type::eScancode: return scancode <=> rhs.scancode;
        case Type::eButtoncode: return buttoncode <=> rhs.buttoncode;
        case Type::eAxiscode: return axiscode <=> rhs.axiscode;
        };
    }
};
static_assert( sizeof( Actuator ) == 8 );
static_assert( alignof( Actuator ) == 4 );

struct Action {
    union {
        float analog = 0.0f;
        bool digital;
    };
    UserEnumUType userEnum{};
    template  <typename T>
    T toA() const noexcept
    {
        return static_cast<T>( userEnum );
    }
};
static_assert( sizeof( Action ) == 8 );
static_assert( alignof( Action ) == 4 );
