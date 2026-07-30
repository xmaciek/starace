#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <shared/stack_vector.hpp>

#include <array>
#include <cstdint>
#include <span>

using PipelineSlot = uint32_t;

struct PipelineCreateInfo {
    enum class Topology : uint8_t { eLineStrip, eLineList, eTriangleFan, eTriangleList, eTriangleStrip };
    enum class CullMode : uint8_t { eNone, eFront, eBack };
    enum class FrontFace : uint8_t { eCW, eCCW };
    enum class InputType : uint8_t { eNone, eF2, eF3, eU, eU2 };
    enum class BlendMode : uint8_t { eNone, eAlpha, eAdditive };
    enum class Stage : uint8_t {
        eVertexUniform,
        eVertexImage,
        eVertexStorage,
        eFragmentUniform,
        eFragmentImage,
        eFragmentStorage,
        eComputeUniform,
        eComputeImage,
        eComputeStorage,
    };
    struct BindInfo {
        Stage m_stage;
        uint8_t m_location;
        uint8_t m_count;
    };

    struct Assembly {
        InputType m_input{};
        uint8_t m_location{};
        uint8_t m_offset{};
    };

    const char* m_vertexShader = nullptr;
    const char* m_fragmentShader = nullptr;
    const char* m_computeShader = nullptr;
    std::span<const uint8_t> m_vertexShaderData{};
    std::span<const uint8_t> m_fragmentShaderData{};
    std::span<const uint8_t> m_computeShaderData{};
    uint32_t m_userHint = 0;
    bool m_enableDepthTest : 1 = false;
    bool m_enableDepthWrite : 1 = false;
    Topology m_topology{};
    CullMode m_cullMode{};
    FrontFace m_frontFace{};
    BlendMode m_blendMode{};
    uint8_t m_vertexStride = 0;
    std::array<Assembly, 3> m_vertexAssembly{};
    StackVector<BindInfo, 8> m_bindInfo{};
};
