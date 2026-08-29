#pragma once

#include <shared/indexer.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>

template <typename T, size_t TCapacity>
requires ( std::is_trivially_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T> )
class Pool : protected Indexer<TCapacity> {
    using Super = Indexer<TCapacity>;
    std::array<T, TCapacity> m_data{};
public:
    using index_type = uint64_t;
    using value_type = T;
    using size_type = size_t;

    virtual ~Pool() noexcept = default;
    Pool() noexcept = default;

    [[nodiscard]]
    index_type emplace( const T& t )
    {
        index_type i = Super::next();
        m_data[ i ] = t;
        return i;
    }

    void release( index_type i )
    {
        Super::release( i );
    }

    T& operator [] ( index_type i )
    {
        assert( i < TCapacity );
        return m_data[ i ];
    }

    const T& operator [] ( index_type i ) const
    {
        assert( i < TCapacity );
        return m_data[ i ];
    }

    size_type size() const { return Super::size(); }

};


