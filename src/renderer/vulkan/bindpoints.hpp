#pragma once

#include <cstdint>
#include <type_traits>
#include <compare>

struct Bindpoints {
    enum class Stage : uint16_t {
        eGraphics,
        eCompute,
    };
    uint16_t constant;
    uint16_t image;
    uint16_t buffer;
    Stage stage;
    inline std::strong_ordering operator <=> ( const Bindpoints& rhs ) const noexcept
    {
        if ( constant != rhs.constant ) return constant <=> rhs.constant;
        if ( image != rhs.image ) return image <=> rhs.image;
        if ( buffer != rhs.buffer ) return buffer <=> rhs.buffer;
        return stage <=> rhs.stage;
    }
};

static_assert( std::is_trivially_constructible_v<Bindpoints> );
static_assert( std::is_trivially_copyable_v<Bindpoints> );
static_assert( std::is_trivially_destructible_v<Bindpoints> );
