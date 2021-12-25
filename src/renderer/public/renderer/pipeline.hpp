#pragma once

#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <vector>

using PipelineSlot = uint16_t;

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
