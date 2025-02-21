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
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
    std::swap( m_pools, rhs.m_pools );
    std::swap( m_uniformCount, rhs.m_uniformCount );
    std::swap( m_imagesCount, rhs.m_imagesCount );
    std::swap( m_imageType, rhs.m_imageType );
}

DescriptorSet& DescriptorSet::operator = ( DescriptorSet&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_set, rhs.m_set );
    std::swap( m_current, rhs.m_current );
    std::swap( m_pools, rhs.m_pools );
    std::swap( m_uniformCount, rhs.m_uniformCount );
    std::swap( m_imagesCount, rhs.m_imagesCount );
    std::swap( m_imageType, rhs.m_imageType );
    return *this;
}

[[maybe_unused]]
static constexpr bool validateDescriptorTypeIsSupportedImage( VkDescriptorType d )
{
    switch ( d ) {
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return true;
    default:
        return false;
    }
}

static VkDescriptorSetLayout createLayout( VkDevice device, const PipelineCreateInfo& pci )
{
    ZoneScoped;
    assert( device );

    std::pmr::vector<VkDescriptorSetLayoutBinding> layoutBinding;
    layoutBinding.reserve( 16 );
    auto push = [&layoutBinding]( uint8_t bindBits, auto type, auto stage )
    {
        uint32_t idxn = 0;
        while ( bindBits ) {
            uint32_t idx = idxn++;
            uint8_t bit = bindBits & 0b1;
            bindBits >>= 1;
            if ( !bit ) continue;
            layoutBinding.emplace_back( VkDescriptorSetLayoutBinding{
                .binding = idx,
                .descriptorType = type,
                .descriptorCount = 1,
                .stageFlags = static_cast<VkShaderStageFlags>( stage ),
            } );
        }
    };
    push( pci.m_vertexUniform, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT );
    push( pci.m_fragmentImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT );
    push( pci.m_computeUniform, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT );
    push( pci.m_computeImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT );

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

DescriptorSet::DescriptorSet( VkDevice device, const PipelineCreateInfo& pci ) noexcept
: m_device{ device }
{
    ZoneScoped;
    m_layout = createLayout( m_device, pci );
    m_uniformCount = (uint32_t)std::popcount( pci.m_vertexUniform ) + (uint32_t)std::popcount( pci.m_computeUniform );
    m_imagesCount = (uint32_t)std::popcount( pci.m_fragmentImage ) + (uint32_t)std::popcount( pci.m_computeImage );
    if ( m_imagesCount > 0 ) {
        m_imageType = pci.m_fragmentImage ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    expandCapacityBy( 50 );
}

void DescriptorSet::expandCapacityBy( uint32_t v )
{
    ZoneScoped;
    assert( m_imagesCount == 0 || validateDescriptorTypeIsSupportedImage( m_imageType ) );
    auto currentCapacity = m_set.size();
    uint32_t poolSizeCount = 0;
    std::array<VkDescriptorPoolSize, 2> poolSizes;
    if ( m_uniformCount ) poolSizes[ poolSizeCount++ ] = VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = v * m_uniformCount };
    if ( m_imagesCount ) poolSizes[ poolSizeCount++ ] = VkDescriptorPoolSize{ .type = m_imageType, .descriptorCount = v * m_imagesCount };
    assert( poolSizeCount != 0 );

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = v,
        .poolSizeCount = poolSizeCount,
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
    ret |= pci.m_vertexUniform; ret <<= 8;
    ret |= pci.m_fragmentImage; ret <<= 8;
    ret |= pci.m_computeUniform; ret <<= 8;
    ret |= pci.m_computeImage;
    return ret;
}

