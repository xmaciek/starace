#pragma once

#include "vk.hpp"

#include <vector>
#include <memory_resource>

class Instance {
    VkInstance m_instance{};
    void* m_dll{};
    std::pmr::vector<const char*> m_layers{};

public:
    ~Instance() noexcept;
    Instance() noexcept = default;
    Instance( std::pmr::vector<const char*> extensions ) noexcept;

    Instance( Instance&& ) noexcept;
    Instance( const Instance& ) = delete;

    Instance& operator = ( Instance&& ) noexcept;
    Instance& operator = ( const Instance& ) = delete;

    operator VkInstance () const noexcept;

    std::pmr::vector<const char*> layers() const noexcept;
    PFN_vkVoidFunction procAddr( const char* ) const noexcept;
};
