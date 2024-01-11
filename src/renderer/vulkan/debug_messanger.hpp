#pragma once

#include "vk.hpp"
#include "instance.hpp"

#if ENABLE_VULKAN_VALIDATION
class DebugMsg {
    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_inst = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT m_destroy = nullptr;

public:
    ~DebugMsg() noexcept;
    DebugMsg( const Instance& ) noexcept;
    DebugMsg() noexcept = default;

    DebugMsg( const DebugMsg& ) = delete;
    DebugMsg& operator = ( const DebugMsg& ) = delete;

    DebugMsg( DebugMsg&& ) noexcept;
    DebugMsg& operator = ( DebugMsg&& ) noexcept;
};

#else
class DebugMsg {
public:
    DebugMsg() noexcept = default;
    inline DebugMsg( const Instance& ) noexcept {};
};
#endif
