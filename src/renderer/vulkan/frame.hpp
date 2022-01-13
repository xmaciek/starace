#pragma once

#include "descriptor_set.hpp"
#include "render_target.hpp"
#include "uniform.hpp"

struct Frame
{
    VkCommandBuffer m_cmdTransfer{};
    VkCommandBuffer m_cmdDepthPrepass{};
    VkCommandBuffer m_cmdRender{};
    RenderTarget m_renderDepthTarget{};
    RenderTarget m_renderTarget{};
    DescriptorSet m_descSetUniform{};
    DescriptorSet m_descSetUniformSampler{};
    Uniform m_uniformBuffer{};
};
