#pragma once

#include <renderer/buffer.hpp>

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
    eTriangleFan2dTextureColor,
    eTriangleFan3dColor,
    eTriangleFan3dTexture,
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
    PushBuffer( std::pmr::memory_resource* ) { }
};

template <>
struct PushBuffer<Pipeline::eLine3dStripColor> {
    Pipeline m_pipeline = Pipeline::eLine3dStripColor;
    std::pmr::vector<glm::vec3> m_vertices{};
    std::pmr::vector<glm::vec4> m_colors{};
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    , m_colors{ allocator }
    {
    }
};

template <>
struct PushBuffer<Pipeline::eTriangleFan2dTextureColor> {
    Pipeline m_pipeline = Pipeline::eTriangleFan2dTextureColor;
    std::pmr::vector<glm::vec2> m_vertices{};
    std::pmr::vector<glm::vec2> m_uv{};
    std::pmr::vector<glm::vec4> m_colors{};
    uint32_t m_texture = 0;

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    , m_uv{ allocator }
    , m_colors{ allocator }
    {
    }
};

template <>
struct PushBuffer<Pipeline::eTriangleFan3dTexture> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dTexture;
    std::pmr::vector<glm::vec3> m_vertices{};
    std::pmr::vector<glm::vec2> m_uv{};
    uint32_t m_texture = 0;

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    , m_uv{ allocator }
    {
    }
};

template <>
struct PushBuffer<Pipeline::eGuiTextureColor1> {
    Pipeline m_pipeline = Pipeline::eGuiTextureColor1;
    uint32_t m_texture = 0;

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* ) { }
};

template <>
struct PushConstant<Pipeline::eGuiTextureColor1> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    std::array<glm::vec2, 4> m_vertices{};
    std::array<glm::vec2, 4> m_uv{};
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
    std::pmr::vector<glm::vec3> m_vertices{};
    float m_lineWidth = 1.0f;

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    {
    }
};

template <>
struct PushBuffer<Pipeline::eTriangle3dTextureNormal> {
    Pipeline m_pipeline = Pipeline::eTriangle3dTextureNormal;
    Buffer m_vertices{};
    Buffer m_normals{};
    Buffer m_uv{};
    uint32_t m_texture = 0;
    PushBuffer() = default;
};

template <>
struct PushBuffer<Pipeline::eTriangleFan3dColor> {
    Pipeline m_pipeline = Pipeline::eTriangleFan3dColor;
    std::pmr::vector<glm::vec3> m_vertices{};
    std::pmr::vector<glm::vec4> m_colors{};

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    , m_colors{ allocator }
    {
    }
};
