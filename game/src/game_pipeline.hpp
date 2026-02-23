#pragma once

#include <renderer/pipeline.hpp>
#include <math.hpp>

enum class Pipeline : PipelineSlot {
    eBackground,
    eMesh,
    eSpaceDust,
    eParticleBlob,
    eThruster,
    eThruster2,
    eBeamBlob,
    eAntiAliasFXAA,
    eProjectile,
    eAfterglow,
    eTail,
    eSkybox,
    count,
};

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eBackground> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    math::vec4 m_uv{};
    math::vec4 m_geometry{};
    math::vec2 m_viewport{};
};

template <>
struct PushConstant<Pipeline::eSpaceDust> {
    static constexpr uint32_t INSTANCES = 100;
    static constexpr uint32_t VERTICES = 2;
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    math::vec4 m_particleOffset{};
    std::array<math::vec4, INSTANCES> m_particles{};
};

template <>
struct PushConstant<Pipeline::eMesh> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
};

template <>
struct PushConstant<Pipeline::eThruster2> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_colorInner1{};
    math::vec4 m_colorInner2{};
    math::vec4 m_colorOutter1{};
    math::vec4 m_colorOutter2{};
};

template <>
struct PushConstant<Pipeline::eAfterglow> {
    static constexpr uint32_t VERTICES = 6;
    static constexpr uint32_t INSTANCES = 4;
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    alignas( 16 ) math::vec3 m_modelOffset{};
    std::array<math::vec4, INSTANCES> m_zSizeCutoff{};
    std::array<math::vec4, 4> m_colorScheme;
};

template <>
struct PushConstant<Pipeline::eSkybox> {
    static constexpr uint32_t VERTICES = 4;
    static constexpr uint32_t INSTANCES = 6;
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
};

template <>
struct PushConstant<Pipeline::eParticleBlob> {
    static constexpr uint32_t INSTANCES = 320;
    static constexpr uint32_t VERTICES = 6;
    struct Instance {
        math::vec4 m_position{};
        math::vec4 m_uvxywh{};
        math::vec4 m_color{};
    };
    math::mat4 m_view{};
    math::mat4 m_projection{};
    alignas( 16 ) math::vec3 m_cameraPosition{};
    alignas( 16 ) math::vec3 m_cameraUp{};
    alignas( 16 ) std::array<Instance, INSTANCES> m_instances{};
};

template <>
struct PushConstant<Pipeline::eTail> {
    static constexpr uint32_t INSTANCES = 320;
    static constexpr uint32_t VERTICES = 16;
    struct Instance {
        std::array<math::vec3, 8> position{};
    };
    math::mat4 m_view{};
    math::mat4 m_projection{};
    alignas( 16 ) math::vec3 m_cameraDirection{};
    alignas( 16 ) math::vec3 m_cameraUp{};
    std::array<Instance, INSTANCES> m_instances{};
};

template <>
struct PushConstant<Pipeline::eProjectile> {
    static constexpr uint32_t INSTANCES = 64;
    static constexpr uint32_t VERTICES = 0;
    struct Instance {
        alignas( 16 ) math::quat m_quat{};
        alignas( 16 ) math::vec4 m_positionScale{};
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<Instance, INSTANCES> m_instances{};
};

template <>
struct PushConstant<Pipeline::eAntiAliasFXAA> {
};

template <>
struct PushConstant<Pipeline::eBeamBlob> {
    static constexpr uint32_t INSTANCES = 3;
    static constexpr uint32_t VERTICES = 12;
    struct Instance {
        alignas( 16 ) math::vec3 m_position{};
        alignas( 16 ) math::quat m_quat{};
        alignas( 16 ) math::vec3 m_displacement{};
        alignas( 16 ) math::vec4 m_color1{};
        alignas( 16 ) math::vec4 m_color2{};
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<Instance, INSTANCES> m_instances{};
};

struct PipelineAtlas {
    std::array<PipelineSlot, static_cast<uint32_t>( Pipeline::count )> m_pipes{};
    constexpr PipelineAtlas() noexcept = default;
    PipelineSlot& operator [] ( Pipeline p ) noexcept
    {
        return m_pipes[ static_cast<uint32_t>( p ) ];
    }
};
[[maybe_unused]]
inline constinit PipelineAtlas g_pipelines{};

