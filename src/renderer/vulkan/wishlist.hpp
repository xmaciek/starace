#pragma once

#include "vk.hpp"

#include <algorithm>
#include <cstring>
#include <memory_resource>
#include <vector>

template <typename T>
struct Wishlist {
    std::pmr::vector<T>* m_checklist = nullptr;
    std::pmr::vector<const char*>* m_ret = nullptr;

    static bool scmp( const VkLayerProperties& prop, const char* name ) { return std::strcmp( prop.layerName, name ) == 0; }
    static bool scmp( const VkExtensionProperties& prop, const char* name ) { return std::strcmp( prop.extensionName, name ) == 0; }

    inline bool operator () ( const char* name )
    {
        auto cmp = [name]( const auto& prop ) { return scmp( prop, name ); };
        if ( std::find_if( m_checklist->begin(), m_checklist->end(), cmp ) == m_checklist->end() ) return false;
        m_ret->emplace_back( name );
        return true;
    }
};
