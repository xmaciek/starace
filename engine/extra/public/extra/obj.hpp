#pragma once

#include <cstdint>
#include <memory_resource>
#include <filesystem>
#include <vector>
#include <utility>

namespace obj {

enum class DataType : uint32_t {
    invalid,
    v = 'v',
    vtn = 'ntv',
};

struct Header {
    static constexpr uint32_t MAGIC = 'CJBO';
    static constexpr uint32_t VERSION = 2;
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t chunkCount = 0;

    Header() noexcept = default;
};

struct Chunk {
    static constexpr uint32_t MAGIC = 'KNHC';
    uint32_t magic = MAGIC;
    char name[ 52 ]{};
    DataType dataType = DataType::invalid;
    uint32_t floatCount = 0;

    Chunk() noexcept = default;
};

std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> load( const std::filesystem::path& );
std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> parse( std::pmr::vector<uint8_t>&& );
} // namespace obj
