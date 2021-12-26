#pragma once

#include <renderer/pipeline.hpp>

static constexpr PipelineCreateInfo g_pipelineGui{
    .m_vertexShader = "shaders/gui_texture_color.vert.spv",
    .m_fragmentShader = "shaders/gui_texture_color.frag.spv",
    .m_slot = static_cast<PipelineSlot>( Pipeline::eGuiTextureColor1 ),
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
