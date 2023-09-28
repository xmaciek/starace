#include "descriptor_set.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

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

static VkDescriptorType guessImageType( const DescriptorSet::BindingInfo& binding )
{
    for ( auto&& it : binding ) {
        switch ( it ) {
        case BindType::eComputeImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case BindType::eFragmentImage: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default:
            continue;
        }
    }
    assert( !"unable to guess desctiptor image type" );
    return {};
}

static VkDescriptorSetLayout createLayout( VkDevice device, const DescriptorSet::BindingInfo& binding )
{
    ZoneScoped;
    assert( device );
    auto convert = []( BindType b, uint32_t idx ) -> VkDescriptorSetLayoutBinding
    {
        switch ( b ) {
        case BindType::eComputeUniform: return {
            .binding = idx,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        };
        case BindType::eComputeImage: return {
            .binding = idx,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        };
        case BindType::eVertexUniform: return {
            .binding = idx,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        };
        case BindType::eFragmentImage: return {
            .binding = idx,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        default:
            assert( !"todo" );
            return {};
        }
    };
    std::pmr::vector<VkDescriptorSetLayoutBinding> layoutBinding;
    layoutBinding.reserve( binding.size() );

    for ( uint32_t i = 0; i < binding.size(); ++i ) {
        if ( binding[ i ] == BindType::none ) continue;
        layoutBinding.push_back( convert( binding[ i ], i ) );
    }

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

DescriptorSet::DescriptorSet( VkDevice device, const BindingInfo& binding ) noexcept
: m_device{ device }
{
    ZoneScoped;
    m_layout = createLayout( m_device, binding );
    auto predicate = []( BindType b ) -> bool { return ( b & BindType::fImage ) == BindType::fImage; };
    m_imagesCount = static_cast<uint32_t>( std::count_if( binding.begin(), binding.end(), predicate ) );
    if ( m_imagesCount > 0 ) {
        m_imageType = guessImageType( binding );
    }
    expandCapacityBy( 50 );
}

void DescriptorSet::expandCapacityBy( uint32_t v )
{
    ZoneScoped;
    assert( m_imagesCount == 0 || validateDescriptorTypeIsSupportedImage( m_imageType ) );
    auto currentCapacity = m_set.size();
    const uint32_t poolSizeCount = 1 + !!m_imagesCount;
    const std::array poolSizes = {
        VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = v },
        VkDescriptorPoolSize{ .type = m_imageType, .descriptorCount = v * m_imagesCount },
    };

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
