#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
    uint32_t m_pushConstantSize = 0;
    uint32_t m_verticeCount = 0;
    float m_lineWidth = 1.0f;
    Buffer m_vertice{};
    Texture m_texture{};
};

enum class Pipeline : PipelineSlot {
    eGuiTextureColor1,
    eLine3dColor1,
    eLine3dStripColor,
    eTriangle3dTextureNormal,
    eTriangleFan3dColor,
    eTriangleFan3dTexture,
    eShortString,
    count,
};

template <Pipeline P>
struct PushConstant {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eLine3dStripColor> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec4, 32> m_vertices{};
    std::array<glm::vec4, 32> m_colors{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eTriangleFan3dTexture> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec4, 4> m_vertices{};
    std::array<glm::vec4, 4> m_uv{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eGuiTextureColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};
    std::array<glm::vec4, 4> m_vertices{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eLine3dColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};
    std::array<glm::vec4, 200> m_vertices{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eTriangleFan3dColor> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec4, 48> m_vertices{};
    std::array<glm::vec4, 48> m_colors{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eShortString> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};

    static constexpr size_t charCount = 48;
    std::array<glm::vec4, charCount * 6> m_vertices{};

    PushConstant() = default;
};
