#pragma once

#include <renderer/pipeline.hpp>
#include <engine/math.hpp>

enum class Pipeline : PipelineSlot {
    eTriangleFan3dTexture,
    eBackground,
    eAlbedo,
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
    math::vec4 m_uvSlice{};
    std::array<math::vec4, 4> m_xyuv{};
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
struct PushConstant<Pipeline::eAlbedo> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
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

[[maybe_unused]]
inline constexpr std::array g_pipelineCreateInfo = {

PipelineCreateInfo{
    .m_vertexShader = "shaders/background.vert.spv",
    .m_fragmentShader = "shaders/background.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eBackground ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eBackground> ),
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/trianglefan_texture.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_texture.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eTriangleFan3dTexture ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dTexture> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/space_dust.vert.spv",
    .m_fragmentShader = "shaders/space_dust.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpaceDust ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpaceDust> ),
    .m_enableDepthTest = true,
    .m_topology = PipelineCreateInfo::Topology::eLineList,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/albedo.vert.spv",
    .m_fragmentShader = "shaders/albedo.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eAlbedo ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eAlbedo> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_vertexStride = sizeof( float ) * 8,
    .m_vertexAssembly{
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 0, 0 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF2, 1, 12 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 2, 20 }
    },
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/mesh.vert.spv",
    .m_fragmentShader = "shaders/mesh.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eMesh ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eMesh> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_vertexStride = sizeof( float ) * 8,
    .m_vertexAssembly{
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 0, 0 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF2, 1, 12 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 2, 20 }
    },
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/thruster2.vert.spv",
    .m_fragmentShader = "shaders/thruster2.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eThruster2 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eThruster2> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexStride = sizeof( float ) * 8,
    .m_vertexAssembly{
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 0, 0 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF2, 1, 12 },
    },
    .m_vertexUniformCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/particles_blob.vert.spv",
    .m_fragmentShader = "shaders/particles_blob.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eParticleBlob ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eParticleBlob> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAdditive,
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/afterglow.vert.spv",
    .m_fragmentShader = "shaders/afterglow.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eAfterglow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eAfterglow> ),
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eNone,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
},

PipelineCreateInfo{
    .m_computeShader = "shaders/gamma.comp.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGammaCorrection ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGammaCorrection> ),
    .m_computeUniformCount = 1,
    .m_computeImageCount = 2,
},

PipelineCreateInfo{
    .m_computeShader = "shaders/antialias_fxaa.comp.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eAntiAliasFXAA ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eAntiAliasFXAA> ),
    .m_computeImageCount = 2,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/beam_blob.vert.spv",
    .m_fragmentShader = "shaders/beam_blob.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eBeamBlob ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eBeamBlob> ),
    .m_enableDepthTest = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eNone,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/projectile.vert.spv",
    .m_fragmentShader = "shaders/projectile.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eProjectile ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eProjectile> ),
    .m_enableDepthTest = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eNone,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexStride = sizeof( float ) * 8,
    .m_vertexAssembly{
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF3, 0, 0 },
        PipelineCreateInfo::Assembly{ PipelineCreateInfo::InputType::eF2, 1, 12 },
    },
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

};
