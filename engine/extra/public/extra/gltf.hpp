#pragma once

#include <cstdint>
#include <string>
#include <memory_resource>

namespace gltf {

enum class Target : uint32_t {
    invalid,
    eVertex = 34962u,
    eIndex = 34963u,
};

enum class Type : uint32_t {
    eScalar,
    eVec2,
    eVec3,
    eVec4,
    eMat2,
    eMat3,
    eMat4,
};

enum class ComponentType : uint32_t {
    invalid,
    eI8 = 5120u,
    eU8 = 5121u,
    eI16 = 5122u,
    eU16 = 5123u,
    eU32 = 5125u,
    eFloat = 5126u,
};

struct BufferView {
    uint32_t buffer = ~0u;
    uint32_t byteOffset = ~0u;
    uint32_t byteLength = ~0u;
    Target target{};
};

struct Accessor {
    uint32_t bufferView = ~0u;
    uint32_t count{};
    Type type{};
    ComponentType componentType{};
    float min[ 3 ]{};
    float max[ 3 ]{};
};

struct Mesh {
    std::pmr::string name{};
    int32_t indices = -1;
    int32_t position = -1;
    int32_t normal = -1;
    int32_t texcoord0 = -1;
    int32_t texcoord1 = -1;
};

struct Node {
    std::pmr::string name{};
    int32_t mesh = -1;
};

}

namespace gltf::glb {

enum Magic : uint32_t {
    eGltf = 'FTlg',
    eJson = 'NOSJ',
    eBin = '\0NIB',
};

struct Header {
    Magic magic{};
    uint32_t version{};
    uint32_t size{};
};

struct Chunk {
    uint32_t size{};
    Magic magic{};
};

}
