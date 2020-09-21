#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <array>
#include <memory_resource>
#include <vector>

enum struct Pipeline {
    eLineStripBlend,
    eTriangleFan2dTextureColor,
    eGuiTextureColor1,
};

template <Pipeline P>
struct PushConstant {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
};

template <Pipeline>
struct PushBuffer;

template <>
struct PushBuffer<Pipeline::eLineStripBlend> {
    Pipeline m_pipeline = Pipeline::eLineStripBlend;
    std::pmr::vector<glm::vec3> m_vertices{};
    std::pmr::vector<glm::vec4> m_colors{};

    PushBuffer() = default;
    PushBuffer( std::pmr::memory_resource* allocator )
    : m_vertices{ allocator }
    , m_colors{ allocator }
    {}
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
    {}
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
};
