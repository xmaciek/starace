#pragma once

#include <cassert>
#include <atomic>
#include <array>
#include <cstddef>

template <typename T, size_t TCapacity>
class Pool {
    using Storage = std::aligned_storage_t<sizeof( T ), alignof( T )>;
    using Atomic = std::atomic<uint64_t>;
    static constexpr size_t c_atomicCount = ( TCapacity >> 6 ) + 1;
    static constexpr size_t c_objectsPerChunk = 64;
    static constexpr uint64_t c_invalidIndex = ~(uint64_t)0;
    static constexpr uint64_t c_bitMax = c_objectsPerChunk - 1;
    static constexpr uint64_t c_bit = 1;
    static_assert( c_atomicCount * c_objectsPerChunk >= TCapacity );

    Atomic m_numAllocs = 0;
    std::array<Atomic, c_atomicCount> m_indexes{};
    std::array<Storage, TCapacity> m_data{};

    static uint64_t pickIndex( Atomic& atomic ) noexcept
    {
        uint64_t oldValue = 0;
        uint64_t candidate = 0;
        uint64_t index = 0;
        do {
            oldValue = atomic.load();
            const uint64_t neg = ~oldValue;
            if ( neg == 0 ) { return c_invalidIndex; }
            index = __builtin_clzll( neg );
            const uint64_t bitToSet = c_bitMax - index;
            const uint64_t bitMask = c_bit << bitToSet;
            assert( __builtin_popcountll( bitMask ) == 1 );
            candidate = oldValue | bitMask;
        } while ( !atomic.compare_exchange_strong( oldValue, candidate ) );
        return index;
    }

    uint64_t pick() noexcept
    {
        uint64_t offsetFromStart = 0;
        for ( Atomic& it : m_indexes ) {
            const uint64_t index = pickIndex( it );
            if ( index != c_invalidIndex ) {
                return index + offsetFromStart;
            }
            offsetFromStart += c_objectsPerChunk;
        }
        return c_invalidIndex;
    }

    static constexpr std::pair<size_t, size_t> split( size_t index ) noexcept
    {
        const uint64_t chunk = index >> 6;
        const uint64_t bit = c_bitMax - ( index & 0b111111 );
        const uint64_t bitMask = c_bit << bit;
        assert( __builtin_popcountll( bitMask ) == 1 );
        return { chunk, bitMask };
    }

public:
    ~Pool() noexcept = default;
    Pool() noexcept = default;

    [[nodiscard]]
    void* alloc() noexcept
    {
        const uint64_t index = pick();
        assert( index < TCapacity );
        if ( index >= TCapacity ) {
            return nullptr;
        }
        assert( index != c_invalidIndex );
        if ( index == c_invalidIndex ) {
            return nullptr;
        }

        [[maybe_unused]]
        const auto [ chunk, bitMask ] = split( index );
        assert( ( m_indexes[ chunk ].load() & bitMask ) == bitMask );
        m_numAllocs.fetch_add( 1 );
        return &m_data[ index ];
    }

    void dealloc( void* ptr ) noexcept
    {
        m_numAllocs.fetch_sub( 1 );
        const size_t index = reinterpret_cast<const Storage*>( ptr ) - &m_data.front();
        assert( index < m_data.size() );
        const auto [ chunk, bitMask ] = split( index );
        assert( ( m_indexes[ chunk ].load() & bitMask ) == bitMask );
        m_indexes[ chunk ].fetch_and( ~bitMask );
    }

    uint64_t allocCount() const noexcept
    {
        return m_numAllocs.load();
    }

    void discardAll() noexcept
    {
        m_numAllocs.store( 0 );
        for ( auto& it : m_indexes ) {
            it = 0;
        }
    }
};
