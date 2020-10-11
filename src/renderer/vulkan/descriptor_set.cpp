#include "descriptor_set.hpp"

#include <cassert>
#include <iostream>

void DescriptorSet::destroyResources()
{
    if ( m_layout ) {
        vkDestroyDescriptorSetLayout( m_device, m_layout, nullptr );
    }
    if ( m_pool ) {
        vkDestroyDescriptorPool( m_device, m_pool, nullptr );
    }
}

DescriptorSet::~DescriptorSet()
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
    m_device = rhs.m_device;
    m_pool = rhs.m_pool;
    m_layout = rhs.m_layout;
    m_set = std::move( rhs.m_set );
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_pool = VK_NULL_HANDLE;
    rhs.m_layout = VK_NULL_HANDLE;
    return *this;
}

DescriptorSet::DescriptorSet( VkDevice device, uint32_t swapchainCount, uint32_t setsPerFrame, const std::pmr::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>>& types )
: m_device{ device }
{
    std::pmr::vector<VkDescriptorSetLayoutBinding> layoutBinding{};
    for ( uint32_t i = 0; i < types.size(); ++i ) {
        layoutBinding.emplace_back(
            VkDescriptorSetLayoutBinding{
                .binding = i,
                .descriptorType = types[ i ].first,
                .descriptorCount = 1,
                .stageFlags = types[ i ].second,
            }
        );
    }

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)layoutBinding.size(),
        .pBindings = layoutBinding.data(),
    };
    if ( const VkResult res = vkCreateDescriptorSetLayout( m_device, &layoutInfo, nullptr, &m_layout );
        res != VK_SUCCESS ) {
        assert( !"failed to create descriptor set layout" );
        std::cout << "failed to create descriptor set layout" << std::endl;
        return;
    }

    std::pmr::vector<VkDescriptorPoolSize> descriptorPoolSize{ types.size() };
    for ( size_t i = 0; i < types.size(); ++i ) {
        descriptorPoolSize[ i ].type = types[ i ].first;
        descriptorPoolSize[ i ].descriptorCount = swapchainCount * setsPerFrame;
    }
    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = swapchainCount * setsPerFrame,
        .poolSizeCount = (uint32_t)descriptorPoolSize.size(),
        .pPoolSizes = descriptorPoolSize.data(),
    };

    if ( const VkResult res = vkCreateDescriptorPool( device, &poolInfo, nullptr, &m_pool );
        res != VK_SUCCESS ) {
        assert( !"failed to create descriptor pool" );
        std::cout << "failed to create descriptor pool" << std::endl;
        return;
    }

    m_set.resize( swapchainCount * setsPerFrame );
    const std::pmr::vector<VkDescriptorSetLayout> layouts( swapchainCount * setsPerFrame, m_layout );
    const VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = swapchainCount * setsPerFrame,
        .pSetLayouts = layouts.data(),
    };

    if ( const VkResult res = vkAllocateDescriptorSets( m_device, &allocInfo, m_set.data() );
        res != VK_SUCCESS ) {
        assert( !"failed to allocate descriptor sets" );
        std::cout << "failed to allocate descriptro sets" << std::endl;
        return;
    }

    for ( VkDescriptorSet it : m_set ) {
        assert( it != VK_NULL_HANDLE );
    }
}


const VkDescriptorSetLayout* DescriptorSet::layout() const
{
    return &m_layout;
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
