#pragma once

#include <SDL_gamecontroller.h>
#include <SDL_scancode.h>

#include <compare>
#include <cstdint>
#include <limits>

namespace input {

struct Actuator {
    // TODO: use custom unified enum, while for now i'll keep these
    using Scancode = SDL_Scancode;
    using Buttoncode = SDL_GameControllerButton;
    using Axiscode = SDL_GameControllerAxis;

    enum class Source : uint16_t {
        eKBM,
        eXBoxOne,
    };

    static constexpr inline uint16_t SCANCODE = 1;
    static constexpr inline uint16_t BUTTONCODE = 2;
    static constexpr inline uint16_t AXISCODE = 3;

    using value_type = int16_t;
    using Limits = std::numeric_limits<value_type>;
    static constexpr inline int16_t MAX = Limits::max();
    static constexpr inline int16_t MIN = Limits::min();
    static constexpr inline int16_t NOMINAL = 0;

    struct Typed {
        uint16_t type : 2;
        uint16_t id : 14;
        static constexpr inline auto MAXID = 0x3FFF;
        static_assert( SDL_NUM_SCANCODES <= MAXID );
        static_assert( SDL_CONTROLLER_BUTTON_MAX <= MAXID );
        static_assert( SDL_CONTROLLER_AXIS_MAX <= MAXID );
    };

    union {
        uint16_t raw = 0;
        Typed typed;
    };
    value_type value = 0;
    Source source = {};

    constexpr Actuator() noexcept = default;
    constexpr Actuator( Scancode s, bool v = false ) noexcept
    : typed{ .type = SCANCODE, .id = (uint16_t)s }
    , value{ v ? MAX : NOMINAL }
    , source{ Source::eKBM }
    {}

    constexpr Actuator( Buttoncode b, bool v = false ) noexcept
    : typed{ .type = BUTTONCODE, .id = (uint16_t)b }
    , value{ v ? MAX : NOMINAL }
    , source{ Source::eXBoxOne }
    {}

    constexpr Actuator( Axiscode a, value_type v = 0 ) noexcept
    : typed{ .type = AXISCODE, .id = (uint16_t)a }
    , value{ v }
    , source{ Source::eXBoxOne }
    {}

    constexpr bool operator == ( const Actuator& rhs ) const noexcept
    {
        return raw == rhs.raw;
    }
};
static_assert( sizeof( Actuator ) == 6 );
static_assert( alignof( Actuator ) == 2 );

}