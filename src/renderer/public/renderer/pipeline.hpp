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


enum struct Pipeline {
    eGuiTextureColor1,
    eGuiQuadColor1,
    eLine3dColor1,
    eLine3dStripColor,
    eLine3dStripColor1,
    eTriangle3dTextureNormal,
    eTriangleFan3dColor,
    eTriangleFan3dTexture,
    count,
};

template <Pipeline P>
struct PushConstant {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    PushConstant() = default;
};

template <Pipeline TP>
struct PushBuffer {
    Pipeline m_pipeline = TP;

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eLine3dStripColor> {
    Pipeline m_pipeline = Pipeline::eLine3dStripColor;
    Buffer m_vertices{};
    Buffer m_colors{};
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eLine3dStripColor1> {
    Pipeline m_pipeline = Pipeline::eLine3dStripColor1;
    Buffer m_vertices{};
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
};

template <>
struct PushConstant<Pipeline::eLine3dStripColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};
    PushConstant() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangleFan3dTexture> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dTexture;
    Buffer m_vertices{};
    Buffer m_uv{};
    Texture m_texture{};

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eGuiTextureColor1> {
    Pipeline m_pipeline = Pipeline::eGuiTextureColor1;
    Texture m_texture{};

    PushBuffer() = default;
};

template <>
struct PushConstant<Pipeline::eGuiTextureColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec4, 4> m_vertices{};
    std::array<glm::vec4, 4> m_uv{};
    glm::vec4 m_color{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eGuiQuadColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec2, 4> m_vertices{};
    glm::vec4 m_color{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eLine3dColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};

    PushConstant() = default;
};

template <>
struct PushBuffer<Pipeline::eLine3dColor1> {
    Pipeline m_pipeline = Pipeline::eLine3dColor1;
    Buffer m_vertices{};
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangle3dTextureNormal> {
    Pipeline m_pipeline = Pipeline::eTriangle3dTextureNormal;
    Buffer m_vertices{};
    Buffer m_normals{};
    Buffer m_uv{};
    Texture m_texture{};

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangleFan3dColor> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dColor;
    Buffer m_vertices{};
    Buffer m_colors{};

    PushBuffer() = default;
};
