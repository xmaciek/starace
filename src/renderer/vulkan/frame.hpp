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
    RenderTarget m_renderTargetTmp{};
    Uniform m_uniformBuffer{};
    std::array<DescriptorSet, 32> m_descriptorSets{};
};
