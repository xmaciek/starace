#pragma once

#include "descriptor_set.hpp"
#include "image.hpp"
#include "uniform.hpp"

struct Frame
{
    enum class State : uint32_t {
        eNone,
        eGraphics,
        eCompute,
    };
    State m_state = State::eNone;
    VkCommandBuffer m_cmdUniform{};
    VkCommandBuffer m_cmdDepthPrepass{};
    VkCommandBuffer m_cmdColorPass{};
    Image m_renderDepthTarget{};
    Image m_renderTarget{};
    Image m_renderTargetTmp{};
    Uniform m_uniformBuffer{};
    std::array<DescriptorSet, 32> m_descriptorSets{};
    CommandPool m_commandPool{};
};
