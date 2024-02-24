#pragma once

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <array>

namespace ui {

enum class Pipeline : uint32_t {
    eSpriteSequence,
    eSpriteSequenceRGBA,
    eSpriteSequenceColors,
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
    };
    math::mat4 m_model{};
    math::mat4 m_view{};
    math::mat4 m_projection{};
    math::vec4 m_color{};
    std::array<Sprite, INSTANCES> m_sprites{};
};

template <>
struct PushConstant<Pipeline::eSpriteSequenceRGBA> {
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


[[maybe_unused]] inline constexpr auto SPRITE_SEQUENCE =
PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence.frag.spv",
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequence> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
};

[[maybe_unused]] inline constexpr auto SPRITE_SEQUENCE_RGBA =
PipelineCreateInfo{
    .m_vertexShader = "shaders/sprite_sequence.vert.spv",
    .m_fragmentShader = "shaders/sprite_sequence_rgba.frag.spv",
    .m_pushConstantSize = sizeof( PushConstant<Pipeline::eSpriteSequenceRGBA> ),
    .m_enableBlend = true,
    .m_topology = PipelineCreateInfo::Topology::eTriangleList,
    .m_cullMode = PipelineCreateInfo::CullMode::eBack,
    .m_frontFace = PipelineCreateInfo::FrontFace::eCCW,
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
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
    .m_binding{
        BindType::eVertexUniform,
        BindType::eFragmentImage,
    },
};


}