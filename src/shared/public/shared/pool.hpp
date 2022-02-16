#pragma once

#include <shared/indexer.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

template <typename T, size_t TCapacity>
class Pool : protected Indexer<TCapacity> {
    using Super = Indexer<TCapacity>;
    using Storage = std::aligned_storage_t<sizeof( T ), alignof( T )>;

    typename Super::Atomic m_numAllocs = 0;
    std::array<Storage, TCapacity> m_data{};

public:
    ~Pool() noexcept = default;
    Pool() noexcept = default;

    [[nodiscard]]
    void* alloc() noexcept
    {
        const uint64_t index = Super::next();
        assert( index < TCapacity );
        if ( index >= TCapacity ) {
            return nullptr;
        }
        assert( index != Super::c_invalidIndex );
        if ( index == Super::c_invalidIndex ) {
            return nullptr;
        }

        [[maybe_unused]]
        const auto [ chunk, bitMask ] = Super::split( index );
        assert( ( Super::m_indexes[ chunk ].load() & bitMask ) == bitMask );
        m_numAllocs.fetch_add( 1 );
        return &m_data[ index ];
    }

    void dealloc( void* ptr ) noexcept
    {
        m_numAllocs.fetch_sub( 1 );
        const std::uintptr_t index = static_cast<std::uintptr_t>(
            reinterpret_cast<const Storage*>( ptr ) - &m_data.front()
        );
        assert( index < m_data.size() );
        Super::release( index );
    }

    uint64_t allocCount() const noexcept
    {
        return m_numAllocs.load();
    }

    void discardAll() noexcept
    {
        m_numAllocs.store( 0 );
        Super::releaseAll();
    }
};
