#include "descriptor_set.hpp"

#include "utils_vk.hpp"

#include <profiler.hpp>

#include <bit>
#include <array>
#include <cassert>
#include <algorithm>

DescriptorSet::~DescriptorSet() noexcept
{
    destroy<vkDestroyDescriptorSetLayout>( m_device, m_layout );
    for ( auto it : m_pools ) {
        destroy<vkDestroyDescriptorPool>( m_device, it );
    }
}

DescriptorSet::DescriptorSet( DescriptorSet&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_pools, rhs.m_pools );
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
    std::swap( m_bindingInfo, rhs.m_bindingInfo );
}

DescriptorSet& DescriptorSet::operator = ( DescriptorSet&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_pools, rhs.m_pools );
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
    std::swap( m_bindingInfo, rhs.m_bindingInfo );
    return *this;
}

static VkDescriptorSetLayoutBinding convertBinding( const PipelineCreateInfo::BindInfo& bindInfo )
{
    VkDescriptorSetLayoutBinding ret{
        .binding = bindInfo.m_location,
        .descriptorCount = bindInfo.m_count,
    };
    auto set = [&ret]( auto stage, auto type )
    {
        ret.stageFlags = stage;
        ret.descriptorType = type;
    };
    using enum PipelineCreateInfo::Stage;
    switch ( bindInfo.m_stage ) {
    case eVertexUniform: set( VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ); break;
    case eVertexImage:   set( VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ); break;
    case eVertexStorage: set( VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ); break;
    case eFragmentUniform: set( VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ); break;
    case eFragmentImage:   set( VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ); break;
    case eFragmentStorage: set( VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ); break;
    case eComputeUniform: set( VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ); break;
    case eComputeImage:   set( VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ); break;
    case eComputeStorage: set( VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ); break;
    default: assert( "invalid pipeline creation binding info" ); break;
    }
    return ret;
};

DescriptorSet::DescriptorSet( VkDevice device, const PipelineCreateInfo& pci ) noexcept
: m_device{ device }
{
    ZoneScoped;
    assert( m_bindingInfo.capacity() >= pci.m_bindInfo.size() );
    m_bindingInfo.resize( pci.m_bindInfo.size() );
    std::ranges::transform( pci.m_bindInfo, m_bindingInfo.begin(), &convertBinding );

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)m_bindingInfo.size(),
        .pBindings = m_bindingInfo.data(),
    };

    [[maybe_unused]]
    const VkResult layoutOK = vkCreateDescriptorSetLayout( device, &layoutInfo, nullptr, &m_layout );
    assert( layoutOK == VK_SUCCESS );

    expandCapacityBy( 50 );
}

void DescriptorSet::expandCapacityBy( uint32_t v )
{
    ZoneScoped;
    auto currentCapacity = m_set.size();
    StackVector<VkDescriptorPoolSize, 8> poolSizes{};
    assert( poolSizes.capacity() >= m_bindingInfo.size() );

    auto convertToPoolSize = [v]( const auto& bindInfo )
    {
        return VkDescriptorPoolSize{
            .type = bindInfo.descriptorType,
            .descriptorCount = v * bindInfo.descriptorCount,
        };
    };
    poolSizes.resize( m_bindingInfo.size() );
    std::ranges::transform( m_bindingInfo, poolSizes.begin(), convertToPoolSize );
    assert( poolSizes.size() != 0 );

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = v,
        .poolSizeCount = (uint32_t)poolSizes.size(),
        .pPoolSizes = poolSizes.data(),
    };

    VkDescriptorPool pool = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult descriptroPoolOK = vkCreateDescriptorPool( m_device, &poolInfo, nullptr, &pool );
    assert( descriptroPoolOK == VK_SUCCESS );
    m_pools.push_back( pool );

    m_set.resize( currentCapacity + v );
    const std::pmr::vector<VkDescriptorSetLayout> layouts( v, m_layout );
    const VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = v,
        .pSetLayouts = layouts.data(),
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateDescriptorSets( m_device, &allocInfo, m_set.data() + currentCapacity );
    assert( allocOK == VK_SUCCESS );
}

VkDescriptorSetLayout DescriptorSet::layout() const
{
    assert( m_layout );
    return m_layout;
}

VkDescriptorSet DescriptorSet::next()
{
    if ( m_current == m_set.size() ) {
        expandCapacityBy( 50 );
    }
    assert( m_current < m_set.size() );
    return m_set[ m_current++ ];
}

void DescriptorSet::reset()
{
    m_current = 0;
}

uint64_t DescriptorSet::createBindingID( const PipelineCreateInfo& pci )
{
    uint64_t ret = 0;
    auto mix = [&ret]( const auto& v )
    {
        ret = std::rotl( ret, 5 ); ret ^= v.m_location;
        ret = std::rotl( ret, 5 ); ret ^= v.m_count;
    };
    std::ranges::for_each( pci.m_bindInfo, mix );
    return ret;
}

