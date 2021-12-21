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

template <Pipeline TP>
struct PushBuffer {
    Pipeline m_pipeline = TP;

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eLine3dStripColor> {
    Pipeline m_pipeline = Pipeline::eLine3dStripColor;
    uint32_t m_verticeCount = 0;
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
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
struct PushBuffer<Pipeline::eTriangleFan3dTexture> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dTexture;
    Texture m_texture{};

    PushBuffer() = default;
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
struct PushConstant<Pipeline::eLine3dColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};
    std::array<glm::vec4, 200> m_vertices{};

    PushConstant() = default;
};

template <>
struct PushBuffer<Pipeline::eLine3dColor1> {
    Pipeline m_pipeline = Pipeline::eLine3dColor1;
    uint32_t m_verticeCount = 0;
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangle3dTextureNormal> {
    Pipeline m_pipeline = Pipeline::eTriangle3dTextureNormal;
    Buffer m_vertices{};
    Texture m_texture{};

    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangleFan3dColor> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dColor;
    uint32_t m_verticeCount = 0;

    PushBuffer() = default;
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
struct PushBuffer<Pipeline::eShortString> {
    Pipeline m_pipeline = Pipeline::eShortString;
    Texture m_texture{};
    uint32_t m_verticeCount = 0;

    PushBuffer() = default;
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
