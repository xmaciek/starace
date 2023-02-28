#pragma once

#include <renderer/pipeline.hpp>
#include <engine/math.hpp>

enum class Pipeline : PipelineSlot {
    eGuiTextureColor1,
    eLine3dColor1,
    eLine3dStripColor,
    eTriangleFan3dColor,
    eTriangleFan3dTexture,
    eSpriteSequence,
    eSpriteSequenceColors,
    eProgressBar,
    eGlow,
    eBackground,
    eAlbedo,
    eSprite3D,
    eSpaceDust,
    eParticleBlob,
    eThruster,
    eGammaCorrection,
    eScanline,
    count,
};

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eLine3dStripColor> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<math::vec4, 32> m_vertices{};
    std::array<math::vec4, 32> m_colors{};
};

template <>
struct PushConstant<Pipeline::eTriangleFan3dTexture> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<math::vec4, 4> m_vertices{};
    std::array<math::vec4, 4> m_uv{};
};

template <>
struct PushConstant<Pipeline::eGuiTextureColor1> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_vertices{};
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
struct PushConstant<Pipeline::eLine3dColor1> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 200> m_vertices{};
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
struct PushConstant<Pipeline::eTriangleFan3dColor> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<math::vec4, 48> m_vertices{};
    std::array<math::vec4, 48> m_colors{};
};

template <>
struct PushConstant<Pipeline::eSpriteSequence> {
    static constexpr uint32_t INSTANCES = 48;
    struct Sprite {
        math::vec4 m_xywh;
        math::vec4 m_uvwh;
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<Sprite, INSTANCES> m_sprites{};
};

template <>
struct PushConstant<Pipeline::eSpriteSequenceColors> {
    static constexpr uint32_t INSTANCES = 48;
    struct Sprite {
        math::vec4 m_color;
        math::vec4 m_xywh;
        math::vec4 m_uvwh;
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<Sprite, INSTANCES> m_sprites{};
};

template <>
struct PushConstant<Pipeline::eProgressBar> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_vertices[ 4 ]{};
    math::vec4 m_color[ 2 ]{};
    math::vec4 m_axis{};
};

template <>
struct PushConstant<Pipeline::eGlow> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_xyuv{};
};

template <>
struct PushConstant<Pipeline::eThruster> {
    struct alignas( 16 ) Afterglow {
        math::vec4 color{};
        math::vec4 xyzs{};
        float radius = 0.0f;
    };
    alignas( 16 ) math::mat4 m_model{};
    alignas( 16 ) math::mat4 m_view{};
    alignas( 16 ) math::mat4 m_projection{};
    alignas( 16 ) std::array<Afterglow, 4> m_afterglow{};
};

template <>
struct PushConstant<Pipeline::eAlbedo> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
};


template <>
struct PushConstant<Pipeline::eSprite3D> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_vertices{};
    std::array<math::vec4, 4> m_uv{};
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
struct PushConstant<Pipeline::eGammaCorrection> {
    float m_power = 2.2f;
};

template <>
struct PushConstant<Pipeline::eScanline> {
    math::vec4 m_power{};
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
    .m_vertexShader = "shaders/gui_texture_color.vert.spv",
    .m_fragmentShader = "shaders/gui_texture_color.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGuiTextureColor1 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGuiTextureColor1> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/background.vert.spv",
    .m_fragmentShader = "shaders/background.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eBackground ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eBackground> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/line3_strip_color.vert.spv",
    .m_fragmentShader = "shaders/line3_strip_color.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eLine3dStripColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dStripColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eLineStrip,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/trianglefan_texture.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_texture.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eTriangleFan3dTexture ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dTexture> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/trianglefan_color.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_color.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eTriangleFan3dColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/lines_color1.vert.spv",
    .m_fragmentShader = "shaders/lines_color1.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eLine3dColor1 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dColor1> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eLineList,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/space_dust.vert.spv",
    .m_fragmentShader = "shaders/lines_color1.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpaceDust ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpaceDust> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_topology = PipelineCreateInfo::Topology::eLineList,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpriteSequence ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequence> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence_colors.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence_colors.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpriteSequenceColors ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequenceColors> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/albedo.vert.spv",
    .m_fragmentShader = "shaders/albedo.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eAlbedo ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eAlbedo> ),
    .m_enableBlend = false,
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
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/progressbar.vert.spv",
    .m_fragmentShader = "shaders/progressbar.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eProgressBar ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eProgressBar> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/glow.vert.spv",
    .m_fragmentShader = "shaders/glow.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGlow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/thruster.vert.spv",
    .m_fragmentShader = "shaders/thruster.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eThruster ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eThruster> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite3d.vert.spv",
    .m_fragmentShader = "shaders/sprite3d.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSprite3D ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSprite3D> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/particles_blob.vert.spv",
    .m_fragmentShader = "shaders/sprite3d.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eParticleBlob ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eParticleBlob> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
},

PipelineCreateInfo{
    .m_computeShader = "shaders/gamma.comp.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGammaCorrection ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGammaCorrection> ),
    .m_binding{
        BindType::eComputeUniform,
        BindType::eComputeImage,
        BindType::eComputeImage,
    },
},

PipelineCreateInfo{
    .m_computeShader = "shaders/scanline.comp.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eScanline ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eScanline> ),
    .m_binding{
        BindType::eComputeUniform,
        BindType::eComputeImage,
        BindType::eComputeImage,
    },
},


};
