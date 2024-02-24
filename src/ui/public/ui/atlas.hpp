#pragma once

#include <engine/math.hpp>
#include <shared/hash.hpp>
#include <shared/fixed_map.hpp>

#include <cstdint>
#include <span>
#include <tuple>

namespace ui {

class Atlas {
public:
    using hash_type = Hash::value_type;
    struct Sprite {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        operator math::vec4 () const noexcept;
        math::vec4 operator / ( const math::vec2& ) const noexcept;
    };

    using SpritePack = std::tuple<hash_type, Sprite>;
    static constexpr inline bool sortCmp( const SpritePack& l, const SpritePack& r ) noexcept
    { return std::get<0>( l ) < std::get<0>( r ); }

private:
    uint16_t m_width = 0;
    uint16_t m_height = 0;
    FixedMap<hash_type, Sprite, 64> m_map{};

public:
    Atlas() noexcept = default;
    Atlas( std::span<const SpritePack>, uint16_t width, uint16_t height ) noexcept;

    math::vec2 extent() const;
    Sprite operator [] ( hash_type ) const noexcept;

};

}
