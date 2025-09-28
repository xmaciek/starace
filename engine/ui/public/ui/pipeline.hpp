#pragma once

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <array>

namespace ui {

enum class Pipeline : uint32_t {
    eSpriteSequence,
    eSpriteSequenceColors,
    eGlow,
    eBlurDesaturate,
};
using enum Pipeline;

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eSpriteSequence> {
    static constexpr uint32_t INSTANCES = 64;
    static constexpr uint32_t VERTICES = 6;
    struct Sprite {
        math::vec4 m_xywh;
        math::vec4 m_uvwh;
        uint32_t m_whichAtlas;
        uint32_t m_sampleRGBA;
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    alignas( 16 ) std::array<Sprite, INSTANCES> m_sprites{};
};

template <>
struct PushConstant<Pipeline::eSpriteSequenceColors> {
    static constexpr uint32_t INSTANCES = 64;
    static constexpr uint32_t VERTICES = 6;
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
struct PushConstant<Pipeline::eGlow> {
    static constexpr uint32_t VERTICES = 4;
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_xyuv{};
};

template <>
struct PushConstant<Pipeline::eBlurDesaturate> {
    alignas( 16 ) uint32_t m_direction{};
};

[[maybe_unused]] inline constexpr std::array PIPELINES = {
PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpriteSequence ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequence> ),
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 8,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence_colors.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence_colors.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eSpriteSequenceColors ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequenceColors> ),
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
    .m_fragmentImageCount = 1,
},

PipelineCreateInfo{
    .m_vertexShader = "shaders/glow.vert.spv",
    .m_fragmentShader = "shaders/glow.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGlow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_blendMode = PipelineCreateInfo::BlendMode::eAlpha,
    .m_vertexUniformCount = 1,
},

PipelineCreateInfo{
    .m_computeShader = "shaders/blur_desaturate.comp.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eBlurDesaturate ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eBlurDesaturate> ),
    .m_computeUniformCount = 1,
    .m_computeImageCount = 2,
},

};

}
