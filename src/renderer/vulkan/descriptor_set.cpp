#include "descriptor_set.hpp"

#include "utils_vk.hpp"

#include <shared/bit_index_iterator.hpp>

#include <Tracy.hpp>

#include <array>
#include <bit>
#include <cassert>

void DescriptorSet::destroyResources()
{
    destroy<vkDestroyDescriptorSetLayout>( m_device, m_layout );
    destroy<vkDestroyDescriptorPool>( m_device, m_pool );
}

DescriptorSet::~DescriptorSet() noexcept
{
    destroyResources();
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
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_pool = std::exchange( rhs.m_pool, {} );
    m_layout = std::exchange( rhs.m_layout, {} );
    m_set = std::move( rhs.m_set );
    m_current = std::exchange( rhs.m_current, {} );
    return *this;
}

static VkDescriptorSetLayout createLayout( VkDevice device, uint16_t constantBindBits, uint16_t samplerBindBits )
{
    assert( device );
    assert( std::popcount( constantBindBits ) == 1 );
    assert( ( constantBindBits & samplerBindBits ) == 0 ); // mutually exclusive bits

    std::pmr::vector<VkDescriptorSetLayoutBinding> layoutBinding{};
    layoutBinding.reserve( std::popcount( samplerBindBits ) + 1 );
    layoutBinding.emplace_back( VkDescriptorSetLayoutBinding{
        .binding = *BitIndexIterator( constantBindBits ),
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    } );

    for ( BitIndexIterator it = samplerBindBits; it; ++it ) {
        layoutBinding.push_back( VkDescriptorSetLayoutBinding{
            .binding = *it,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        } );
    }

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)layoutBinding.size(),
        .pBindings = layoutBinding.data(),
    };

    VkDescriptorSetLayout ret = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult setLayoutOK = vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &ret );
    assert( setLayoutOK == VK_SUCCESS );
    return ret;
}

DescriptorSet::DescriptorSet(
    VkDevice device
    , uint32_t setsPerFrame
    , uint16_t constantBindBits
    , uint16_t samplerBindBits
) noexcept
: m_device{ device }
{
    ZoneScoped;
    assert( device );
    assert( std::popcount( constantBindBits ) == 1 );
    assert( ( constantBindBits & samplerBindBits ) == 0 ); // mutually exclusive bits

    m_layout = createLayout( m_device, constantBindBits, samplerBindBits );

    const uint32_t poolSizeCount = 1 + ( samplerBindBits != 0 );
    const std::array poolSizes = {
        VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = setsPerFrame },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = setsPerFrame * std::popcount( samplerBindBits ),
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
