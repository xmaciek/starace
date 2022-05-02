#pragma once

#include "descriptor_set.hpp"
#include "render_target.hpp"
#include "uniform.hpp"

struct Frame
{
    enum class State : uint32_t {
        eNone,
        eGraphics,
        eCompute,
    };
    State m_state = State::eNone;
    VkCommandBuffer m_cmdTransfer{};
    VkCommandBuffer m_cmdDepthPrepass{};
    VkCommandBuffer m_cmdRender{};
    RenderTarget m_renderDepthTarget{};
    RenderTarget m_renderTarget{};
    DescriptorSet m_descSetUniform{};
    DescriptorSet m_descSetUniformSampler{};
    DescriptorSet m_descSetUniformImage{};
    Uniform m_uniformBuffer{};
};
