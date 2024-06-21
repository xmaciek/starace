#pragma once

#include <cstdint>
#include <span>

namespace ui {

class Remapper {
public:
    ~Remapper() = default;
    Remapper() = default;
    uint32_t apply( char32_t, std::span<char32_t> ) const;

};

}
