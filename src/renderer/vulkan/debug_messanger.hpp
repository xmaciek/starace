#pragma once

#include "vk.hpp"

class DebugMsg {
    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_inst = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT m_destroy = nullptr;

public:
    ~DebugMsg() noexcept;
    explicit DebugMsg( VkInstance ) noexcept;
    DebugMsg() noexcept = default;

    DebugMsg( const DebugMsg& ) = delete;
    DebugMsg& operator = ( const DebugMsg& ) = delete;

    DebugMsg( DebugMsg&& ) noexcept;
    DebugMsg& operator = ( DebugMsg&& ) noexcept;
};
