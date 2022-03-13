#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <filesystem>
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
    std::filesystem::path previewPath{};
    std::array<std::filesystem::path,Wall::max> filePath{};
    std::array<Texture, 6> texture{};
    Texture preview{};
    uint32_t enemies = 0;
};
