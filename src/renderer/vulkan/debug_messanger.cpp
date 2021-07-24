#include "debug_messanger.hpp"

#include <cassert>
#include <iostream>

DebugMsg::DebugMsg( VkInstance instance ) noexcept
: m_vkInstance{ instance }
{
    assert( instance );
    static constexpr VkDebugUtilsMessengerCreateInfoEXT debugMsg{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity
            = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType
            = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &DebugMsg::callback,
    };
    const PFN_vkCreateDebugUtilsMessengerEXT create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" ) );
    assert( create );
    m_destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" ) );
    assert( m_destroy );

    [[maybe_unused]]
    const VkResult createOK = create( instance, &debugMsg, nullptr, &m_inst );
    assert( createOK == VK_SUCCESS );
}

DebugMsg::~DebugMsg() noexcept
{
    if ( !m_inst ) { return; }
    assert( m_destroy );
    m_destroy( m_vkInstance, m_inst, nullptr );
}

DebugMsg::DebugMsg( DebugMsg&& rhs ) noexcept
{
    std::swap( m_vkInstance, rhs.m_vkInstance );
    std::swap( m_inst, rhs.m_inst );
    std::swap( m_destroy, rhs.m_destroy );
}

DebugMsg& DebugMsg::operator = ( DebugMsg&& rhs ) noexcept
{
    if ( m_inst ) {
        assert( m_vkInstance );
        assert( m_destroy );
        m_destroy( m_vkInstance, m_inst, nullptr );
    }
    m_vkInstance = rhs.m_vkInstance;
    m_inst = rhs.m_inst;
    m_destroy = rhs.m_destroy;
    rhs.m_vkInstance = VK_NULL_HANDLE;
    rhs.m_inst = VK_NULL_HANDLE;
    rhs.m_destroy = nullptr;
    return *this;
}


VkBool32 DebugMsg::callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity
    , VkDebugUtilsMessageTypeFlagsEXT type
    , const VkDebugUtilsMessengerCallbackDataEXT* data
    , void* )
{
    static constexpr uint32_t expectedSeverity
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    if ( !( severity & expectedSeverity ) ) {
        return VK_FALSE;
    }

    static constexpr uint32_t expectedType
        = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    if ( !( type & expectedType ) ) {
        return VK_FALSE;
    }

    std::cout << "[ FAIL ] " << data->pMessage << std::endl << std::flush;
    std::abort();
    return VK_FALSE;
}
