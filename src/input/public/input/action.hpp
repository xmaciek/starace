#pragma once

#include <input/actuator.hpp>
#include <cstdint>

namespace input {

    struct Action {
    using Enum = uint16_t;
    Enum userEnum{};
    Actuator::value_type value{};

    template <Enum TMin, Enum TMax>
    inline bool testEnumRange() const
    {
        return userEnum >= TMin && userEnum < TMax;
    }

    template <typename T>
    inline T toA() const noexcept
    {
        return static_cast<T>( userEnum );
    }

    inline bool digital() const
    {
        return value != 0;
    }

    inline float analog() const
    {
        return value > 0
            ? static_cast<float>( value ) / static_cast<float>( Actuator::MAX )
            : -static_cast<float>( value ) / static_cast<float>( Actuator::MIN );
    }
};
static_assert( sizeof( Action ) == 4 );
static_assert( alignof( Action ) == 2 );

}