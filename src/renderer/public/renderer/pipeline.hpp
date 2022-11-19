#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <array>
#include <cstdint>

using PipelineSlot = uint16_t;

enum class BindType : uint8_t
{
    none = 0u,
    fVertex =   0b1u,
    fFragment = 0b10u,
    fCompute =  0b100u,
    fUniform =  0b1'0000u,
    fImage =    0b10'0000u,

    eVertexUniform = fVertex | fUniform,
    eFragmentImage = fFragment | fImage,
    eComputeUniform = fCompute | fUniform,
    eComputeImage = fCompute | fImage,
};
constexpr BindType operator & ( BindType a, BindType b ) noexcept
{
    using U = std::underlying_type_t<BindType>;
    return static_cast<BindType>( static_cast<U>( a ) & static_cast<U>( b ) );
}

union BindResource {
    void* uniform = nullptr;
    Texture texture;
};

struct PipelineCreateInfo {
    enum class Topology : uint8_t { eLineStrip, eLineList, eTriangleFan, eTriangleList, };
    enum class CullMode : uint8_t { eNone, eFront, eBack };
    enum class FrontFace : uint8_t { eCW, eCCW };
    enum class InputType : uint8_t { eNone, eF2, eF3 };
    struct Assembly {
        InputType m_input{};
        uint8_t m_location{};
        uint8_t m_offset{};
    };

    const char* m_vertexShader = nullptr;
    const char* m_fragmentShader = nullptr;
    const char* m_computeShader = nullptr;
    PipelineSlot m_slot = 0;
    uint32_t m_pushConstantSize = 0;
    bool m_enableBlend : 1 = false;
    bool m_enableDepthTest : 1 = false;
    bool m_enableDepthWrite : 1 = false;
    Topology m_topology{};
    CullMode m_cullMode{};
    FrontFace m_frontFace{};
    uint8_t m_vertexStride = 0;
    std::array<Assembly, 3> m_vertexAssembly{};
    std::array<BindType, 8> m_binding{};
};


struct PushData {
    PipelineSlot m_pipeline{};
    uint32_t m_verticeCount = 0;
    float m_lineWidth = 1.0f;
    Buffer m_vertice{};
    std::array<BindResource, 8> m_resource{};
};

using PushBuffer = PushData;
