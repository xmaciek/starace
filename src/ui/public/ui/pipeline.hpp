#pragma once

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <array>

namespace ui {

enum class Pipeline : uint32_t {
    eSpriteSequence,
    eSpriteSequenceColors,
    eGlow,
};
using enum Pipeline;

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eSpriteSequence> {
    static constexpr uint32_t INSTANCES = 48;
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
struct PushConstant<Pipeline::eGlow> {
    static constexpr uint32_t VERTICES = 4;
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<math::vec4, 4> m_xyuv{};
};

[[maybe_unused]] inline constexpr auto SPRITE_SEQUENCE =
PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence.frag.spv",
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequence> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_vertexUniform = 0b1,
    .m_fragmentImage = 0b110,
};

[[maybe_unused]] inline constexpr auto SPRITE_SEQUENCE_COLORS =
PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence_colors.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence_colors.frag.spv",
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequenceColors> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_vertexUniform = 0b1,
    .m_fragmentImage = 0b10,
};

[[maybe_unused]] inline constexpr auto GLOW =
PipelineCreateInfo{
    .m_vertexShader = "shaders/glow.vert.spv",
    .m_fragmentShader = "shaders/glow.frag.spv",
    .m_userHint = static_cast<uint32_t>( Pipeline::eGlow ),
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eGlow> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleFan,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_vertexUniform = 0b1,
};
}
