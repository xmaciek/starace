#pragma once

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <vector>
#include <utility>
#include <type_traits>

VkFormat pickSupportedFormat( VkPhysicalDevice, const std::pmr::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags );

uint32_t memoryType( VkPhysicalDevice, uint32_t typeBits, VkMemoryPropertyFlags );

template <auto pfnDestroy, typename T>
void destroy( VkDevice device, T t ) noexcept
{
    if ( t ) {
        pfnDestroy( device, t, nullptr );
    }
}

struct TransferInfo {
    VkImageLayout m_layout{};
    VkAccessFlags m_access{};
    VkPipelineStageFlags m_stage{};
    inline bool operator == ( const TransferInfo& ) const noexcept = default;
};

void transferImage( VkCommandBuffer, VkImage, const TransferInfo& src, const TransferInfo& dst );

namespace constants {

static constexpr TransferInfo undefined{
    .m_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    .m_access = 0,
    .m_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
};

static constexpr TransferInfo fragmentWrite{
    .m_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .m_access = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .m_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
};


static constexpr TransferInfo computeReadWrite{
    .m_layout = VK_IMAGE_LAYOUT_GENERAL,
    .m_access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
    .m_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
};


static constexpr TransferInfo present{
    .m_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    .m_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .m_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
};

static constexpr TransferInfo copyTo{
    .m_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .m_access = VK_ACCESS_TRANSFER_WRITE_BIT,
    .m_stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
};

static constexpr TransferInfo copyFrom{
    .m_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    .m_access = VK_ACCESS_TRANSFER_WRITE_BIT,
    .m_stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
};

static constexpr TransferInfo fragmentRead{
    .m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    .m_access = VK_ACCESS_SHADER_READ_BIT,
    .m_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
};

static constexpr TransferInfo depthWrite{
    .m_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    .m_access = 0,
    .m_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
};

static constexpr TransferInfo depthRead{
    .m_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    .m_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    .m_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
};

} // namespace constants
