#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <memory_resource>
#include <string>

struct MapCreateInfo {
    enum Wall {
        eTop,
        eBottom,
        eLeft,
        eRight,
        eFront,
        eBack,
        max,
    };

    std::pmr::u32string name = U"unnamed map";
    std::array<Texture, 6> texture{};
    Texture preview{};
    uint32_t enemies = 0;
};
