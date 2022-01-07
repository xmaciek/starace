#pragma once

#include <renderer/pipeline.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

enum class Pipeline : PipelineSlot {
    eGuiTextureColor1,
    eLine3dColor1,
    eLine3dStripColor,
    eTriangle3dTextureNormal,
    eTriangleFan3dColor,
    eTriangleFan3dTexture,
    eShortString,
    eProgressBar,
    eGlow,
    eBackground,
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
struct PushConstant<Pipeline::eBackground> {
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

template <>
struct PushConstant<Pipeline::eProgressBar> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_vertices[ 4 ]{};
    glm::vec4 m_color[ 2 ]{};
    glm::vec4 m_axis{};

    PushConstant() = default;
};

template <>
struct PushConstant<Pipeline::eGlow> {
    glm::mat4 m_model{};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
    glm::vec4 m_color{};
    std::array<glm::vec4, 4> m_xyuv{};

    PushConstant() = default;
};

static constexpr PipelineCreateInfo g_pipelineGui{
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
};

static constexpr PipelineCreateInfo g_pipelineBackground{
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
};

static constexpr PipelineCreateInfo g_pipelineLineStripColor{
    .m_vertexShader = "shaders/line3_strip_color.vert.spv",
    .m_fragmentShader = "shaders/line3_strip_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eLine3dStripColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dStripColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eLineStrip,
    .m_constantBindBits = 0b1,
};

static constexpr PipelineCreateInfo g_pipelineTriangleFan3DTexture{
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
};

static constexpr PipelineCreateInfo g_pipelineTriangleFan3DColor{
    .m_vertexShader = "shaders/trianglefan_color.vert.spv",
    .m_fragmentShader = "shaders/trianglefan_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eTriangleFan3dColor ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangleFan3dColor> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
};

static constexpr PipelineCreateInfo g_pipelineLine3DColor{
    .m_vertexShader = "shaders/lines_color1.vert.spv",
    .m_fragmentShader = "shaders/lines_color1.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eLine3dColor1 ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eLine3dColor1> ),
    .m_enableBlend = true,
    .m_enableDepthTest = true,
    .m_enableDepthWrite = true,
    .m_topology = PipelineCreateInfo::Topology::eLineList,
    .m_constantBindBits = 0b1,
};

static constexpr PipelineCreateInfo g_pipelineShortString{
    .m_vertexShader = "shaders/short_string.vert.spv",
    .m_fragmentShader = "shaders/short_string.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eShortString ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eShortString> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
    .m_textureBindBits = 0b10,
};

static constexpr PipelineCreateInfo g_pipelineTriangle3DTextureNormal{
    .m_vertexShader = "shaders/vert3_texture_normal3.vert.spv",
    .m_fragmentShader = "shaders/vert3_texture_normal3.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eTriangle3dTextureNormal ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> ),
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
};

static constexpr PipelineCreateInfo g_pipelineProgressBar{
    .m_vertexShader = "shaders/progressbar.vert.spv",
    .m_fragmentShader = "shaders/progressbar.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eProgressBar ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eProgressBar> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
};

static constexpr PipelineCreateInfo g_pipelineGlow{
    .m_vertexShader = "shaders/glow.vert.spv",
    .m_fragmentShader = "shaders/glow.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eGlow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_constantBindBits = 0b1,
};
