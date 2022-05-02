#include "descriptor_set.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

#include <array>
#include <bit>
#include <cassert>

DescriptorSet::~DescriptorSet() noexcept
{
    destroy<vkDestroyDescriptorSetLayout>( m_device, m_layout );
    destroy<vkDestroyDescriptorPool>( m_device, m_pool );
}


DescriptorSet::DescriptorSet( DescriptorSet&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_pool, rhs.m_pool );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
}

DescriptorSet& DescriptorSet::operator = ( DescriptorSet&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_pool, rhs.m_pool );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
    return *this;
}

enum class LayoutStage {
    eGraphics,
    eCompute,
};

static VkDescriptorSetLayout createLayout(
    VkDevice device
    , LayoutStage layoutStage
    , uint16_t constantBindBits
    , uint16_t imageBindBits
)
{
    assert( device );
    assert( std::popcount( constantBindBits ) == 1 );
    assert( ( constantBindBits & imageBindBits ) == 0 ); // mutually exclusive bits

    const auto reserve = std::popcount( constantBindBits ) + std::popcount( imageBindBits );
    std::pmr::vector<VkDescriptorSetLayoutBinding> layoutBinding{ static_cast<std::size_t>( reserve ) };

    struct MakeBinding {
        uint16_t bindBits = 0;
        VkDescriptorType type{};
        VkShaderStageFlagBits stageFlags{};

        VkDescriptorSetLayoutBinding operator () () noexcept
        {
            assert( bindBits != 0 );
            auto binding = std::countr_zero( bindBits );
            bindBits &= ~( 1ull << binding );
            return {
                .binding = static_cast<uint32_t>( binding ),
                .descriptorType = type,
                .descriptorCount = 1,
                .stageFlags = stageFlags,
            };
        }
    };

    auto constantStage = layoutStage == LayoutStage::eGraphics
        ? VK_SHADER_STAGE_VERTEX_BIT
        : VK_SHADER_STAGE_COMPUTE_BIT;
    auto imageStage = layoutStage == LayoutStage::eGraphics
        ? VK_SHADER_STAGE_FRAGMENT_BIT
        : VK_SHADER_STAGE_COMPUTE_BIT;
    auto imageType = layoutStage == LayoutStage::eGraphics
        ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    auto it = layoutBinding.begin();

    it = std::generate_n( it, std::popcount( constantBindBits ),
            MakeBinding{ constantBindBits, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, constantStage } );

    it = std::generate_n( it, std::popcount( imageBindBits ),
            MakeBinding{ imageBindBits, imageType, imageStage } );

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)layoutBinding.size(),
        .pBindings = layoutBinding.data(),
    };

    VkDescriptorSetLayout ret = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult layoutOK = vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &ret );
    assert( layoutOK == VK_SUCCESS );
    return ret;
}

DescriptorSet::DescriptorSet(
    VkDevice device
    , uint32_t setsPerFrame
    , uint16_t constantBindBits
    , uint16_t samplerBindBits
    , uint16_t computeImageBindBits
) noexcept
: m_device{ device }
{
    ZoneScoped;
    assert( device );
    assert( std::popcount( constantBindBits ) == 1 );
    assert( ( constantBindBits & samplerBindBits ) == 0 ); // mutually exclusive bits
    assert( ( constantBindBits & computeImageBindBits ) == 0 ); // mutually exclusive bits
    [[maybe_unused]]
    const bool hasImageBindBits = samplerBindBits || computeImageBindBits;
    assert( !hasImageBindBits || !!samplerBindBits != !!computeImageBindBits ); // mutually exclusive

    if ( computeImageBindBits ) {
        m_layout = createLayout( m_device, LayoutStage::eCompute, constantBindBits, computeImageBindBits );
    }
    else {
        m_layout = createLayout( m_device, LayoutStage::eGraphics, constantBindBits, samplerBindBits );
    }

    const uint32_t poolSizeCount = 1 + ( samplerBindBits != 0 );
    const std::array poolSizes = {
        VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = setsPerFrame },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = setsPerFrame * static_cast<uint32_t>( std::popcount( samplerBindBits ) ),
        }
    };

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = setsPerFrame,
        .poolSizeCount = poolSizeCount,
        .pPoolSizes = poolSizes.data(),
    };

    [[maybe_unused]]
    const VkResult descriptroPoolOK = vkCreateDescriptorPool( device, &poolInfo, nullptr, &m_pool );
    assert( descriptroPoolOK == VK_SUCCESS );

    m_set.resize( setsPerFrame );
    const std::pmr::vector<VkDescriptorSetLayout> layouts( setsPerFrame, m_layout );
    const VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = setsPerFrame,
        .pSetLayouts = layouts.data(),
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateDescriptorSets( m_device, &allocInfo, m_set.data() );
    assert( allocOK == VK_SUCCESS );
}


VkDescriptorSetLayout DescriptorSet::layout() const
{
    return m_layout;
}

VkDescriptorSet DescriptorSet::next()
{
    assert( m_current < m_set.size() );
    return m_set[ m_current++ ];
}

void DescriptorSet::reset()
{
    m_current = 0;
}
