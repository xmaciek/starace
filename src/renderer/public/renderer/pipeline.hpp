#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <array>
#include <cstdint>

using PipelineSlot = uint16_t;

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
    uint16_t m_constantBindBits{};
    uint16_t m_textureBindBits{};
};

struct PushBuffer {
    PipelineSlot m_pipeline{};
    uint16_t m_useLineWidth : 1 = false;
    uint32_t m_verticeCount = 0;
    float m_lineWidth = 1.0f;
    Buffer m_vertice{};
    Texture m_texture{};
};
