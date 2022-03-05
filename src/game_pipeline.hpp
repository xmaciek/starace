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
    eProgressBar,
    eGlow,
    eBackground,
    eAlbedo,
    eSprite3D,
    eThruster,
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
struct PushConstant<Pipeline::eTriangleFan3dColor> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    std::array<math::vec4, 48> m_vertices{};
    std::array<math::vec4, 48> m_colors{};
};

template <>
struct PushConstant<Pipeline::eSpriteSequence> {
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};

    static constexpr size_t c_spriteCount = 48;
    std::array<math::vec4, c_spriteCount * 6> m_xyuv{};
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
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_xyuv{};
    float m_radius = 0.0f;
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

static constexpr std::array g_pipelines = {
PipelineCreateInfo{
    .m_vertexShader = "shaders/gui_texture_color.vert.spv",
    .m_fragmentShader = "shaders/gui_texture_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eGuiTextureColor1 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGuiTextureColor1> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/background.vert.spv",
    .m_fragmentShader = "shaders/background.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eBackground ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eBackground> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/line3_strip_color.vert.spv",
    .m_fragmentShader = "shaders/line3_strip_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eLine3dStripColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dStripColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eLineStrip,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/trianglefan_texture.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_texture.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dTexture ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dTexture> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/trianglefan_color.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/lines_color1.vert.spv",
    .m_fragmentShader = "shaders/lines_color1.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eLine3dColor1 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dColor1> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eLineList,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eSpriteSequence ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequence> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/albedo.vert.spv",
    .m_fragmentShader = "shaders/albedo.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eAlbedo ),
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
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/progressbar.vert.spv",
    .m_fragmentShader = "shaders/progressbar.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eProgressBar ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eProgressBar> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/glow.vert.spv",
    .m_fragmentShader = "shaders/glow.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eGlow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/thruster.vert.spv",
    .m_fragmentShader = "shaders/thruster.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eThruster ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eThruster> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite3d.vert.spv",
    .m_fragmentShader = "shaders/sprite3d.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eSprite3D ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSprite3D> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = false,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
},

};
