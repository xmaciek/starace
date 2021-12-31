#pragma once

#include "descriptor_set.hpp"
#include "render_target.hpp"
#include "uniform.hpp"

struct Frame
{
    RenderTarget m_renderTarget{};
    VkCommandBuffer m_cmdRender{};
    VkCommandBuffer m_cmdTransfer{};
    DescriptorSet m_descSetUniform{};
    DescriptorSet m_descSetUniformSampler{};
    Uniform m_uniformBuffer{};
};
