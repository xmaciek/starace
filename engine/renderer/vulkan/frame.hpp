#pragma once

#include "vk.hpp"
#include "descriptor_set.hpp"
#include "image.hpp"
#include "uniform.hpp"
#include "command_pool.hpp"
#include "renderpass.hpp"

#include <vector>
#include <memory_resource>
#include <optional>

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
    RenderPass m_depthPrepass{};
    RenderPass m_mainPass{};
    std::array<DescriptorSet, 32> m_descriptorSets{};
    std::optional<VkRect2D> m_pendingScissor{};
    std::pmr::vector<VkRect2D> m_scissorStack{};
    CommandPool m_commandPool{};

    void reset();
    void processPendingOperations();
    void pushScissor( const VkRect2D& );
    void popScissor();
    void switchState( State );
};
