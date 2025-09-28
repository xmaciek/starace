#pragma once

#include <shared/hash.hpp>

#include <map>
#include <memory_resource>
#include <string_view>
#include <utility>

template <typename T>
class ResourceMap {
    std::pmr::map<Hash::value_type, T> m_map{};

public:
    auto insert( std::pair<std::string_view, T>&& p )
    {
        return m_map.insert( std::make_pair( Hash{}( p.first ), p.second ) );
    }

    auto insert( std::pair<Hash::value_type, T>&& p )
    {
        return m_map.insert( p );
    }

    T operator [] ( std::string_view sv ) const
    {
        auto it = m_map.find( Hash{}( sv ) );
        return it == m_map.end() ? T{} : it->second;
    }

    T operator [] ( Hash::value_type h ) const
    {
        auto it = m_map.find( h );
        return it == m_map.end() ? T{} : it->second;
    }

};
