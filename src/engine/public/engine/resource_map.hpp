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
    auto insert( std::pair<std::string_view, Hash::value_type>&& p )
    {
        return m_map.insert( std::make_pair( Hash{}( p.first ), p.second ) );
    }

    T operator [] ( std::string_view sv )
    {
        return m_map[ Hash{}( sv ) ];
    }

};
