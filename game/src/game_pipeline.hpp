#pragma once

#include <renderer/pipeline.hpp>
#include <engine/math.hpp>

enum class Pipeline : PipelineSlot {
    eTriangleFan3dTexture,
    eBackground,
    eMesh,
    eSpaceDust,
    eParticleBlob,
    eThruster,
    eThruster2,
    eGammaCorrection,
    eBeamBlob,
    eAntiAliasFXAA,
    eProjectile,
    eAfterglow,
    count,
};

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eTriangleFan3dTexture> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<math::vec4, 4> m_vertices{};
    std::array<math::vec4, 4> m_uv{};
};

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
struct PushConstant<Pipeline::eParticleBlob> {
    static constexpr uint32_t INSTANCES = 320;
    static constexpr uint32_t VERTICES = 6;
    struct Particle {
        alignas( 16 ) math::vec4 m_position{};
        alignas( 16 ) math::vec4 m_uvxywh{};
        alignas( 16 ) math::vec4 m_color{};
    };
    math::mat4 m_view{};
    math::mat4 m_projection{};
    alignas( 16 ) math::vec3 m_cameraPosition{};
    alignas( 16 ) math::vec3 m_cameraUp{};
    alignas( 16 ) std::array<Particle, INSTANCES> m_particles{};
};

template <>
struct PushConstant<Pipeline::eProjectile> {
    static constexpr uint32_t INSTANCES = 64;
    struct Projectile {
        alignas( 16 ) math::quat m_quat{};
        alignas( 16 ) math::vec4 m_positionScale{};
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<Projectile, INSTANCES> m_projectiles{};
};

template <>
struct PushConstant<Pipeline::eGammaCorrection> {
    float m_power = 2.2f;
};

template <>
struct PushConstant<Pipeline::eAntiAliasFXAA> {
};

template <>
struct PushConstant<Pipeline::eBeamBlob> {
    static constexpr uint32_t INSTANCES = 320;
    struct Beam {
        alignas( 16 ) math::vec3 m_position{};
        alignas( 16 ) math::quat m_quat{};
        alignas( 16 ) math::vec3 m_displacement{};
        alignas( 16 ) math::vec4 m_color1{};
        alignas( 16 ) math::vec4 m_color2{};
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<Beam, INSTANCES> m_beams{};
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

