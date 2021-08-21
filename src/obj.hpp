#pragma once

#include <cstdint>
#include <memory_resource>
#include <filesystem>
#include <vector>
#include <utility>

namespace obj {

constexpr uint32_t operator ""_magic( const char* str, size_t len ) noexcept
{
    uint32_t ret = 0;
    switch ( len ) {
    default:
    case 4:
        ret |= str[ 3 ];
    case 3:
        ret <<= 8;
        ret |= str[ 2 ];
    case 2:
        ret <<= 8;
        ret |= str[ 1 ];
    case 1:
        ret <<= 8;
        ret |= str[ 0 ];
    case 0:
        break;
    }
    return ret;
}

enum class DataType : uint32_t {
    invalid,
    v = 'v',
    vtn = "vtn"_magic,
};

struct Header {
    static constexpr uint32_t c_magic = "OBJC"_magic;
    static constexpr uint32_t c_currentVersion = 1;
    uint32_t magic = c_magic;
    uint32_t version = c_currentVersion;
    uint32_t chunkCount = 0;

    Header() noexcept = default;
};

struct Chunk {
    static constexpr uint32_t c_magic = "CHNK"_magic;
    uint32_t magic = c_magic;
    char name[ 16 ]{};
    DataType dataType = DataType::invalid;
    uint32_t floatCount = 0;

    Chunk() noexcept = default;
};

std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> load( const std::filesystem::path& );

} // namespace obj
