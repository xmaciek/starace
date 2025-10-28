#pragma once

#include <engine/math.hpp>
#include <renderer/renderer.hpp>

#include <array>

namespace ui {

enum class Pipeline : uint32_t {
    eSpriteSequence,
    eSpriteSequenceColors,
    eGlow,
    eBlur,
};
using enum Pipeline;

template <Pipeline P>
struct PushConstant;

template <>
struct PushConstant<Pipeline::eSpriteSequence> {
    static constexpr uint32_t INSTANCES = 64;
    static constexpr uint32_t VERTICES = 4;
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
    static constexpr uint32_t VERTICES = 4;
    struct Sprite {
        math::vec4 m_color;
        math::vec4 m_xywh;
        math::vec4 m_uvwh;
        uint32_t m_whichAtlas;
        uint32_t m_sampleRGBA;
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
struct PushConstant<Pipeline::eBlur> {
    alignas( 16 ) uint32_t m_direction{};
};

}
