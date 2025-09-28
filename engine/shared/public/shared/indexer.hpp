#pragma once

#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>

template <size_t TCapacity>
class Indexer {
protected:
    using Atomic = std::atomic<uint64_t>;
    static constexpr uint64_t c_invalidIndex = ~(uint64_t)0;
    static constexpr size_t c_atomicCount = ( TCapacity >> 6 ) + 1;
    static constexpr size_t c_objectsPerChunk = 64;
    static constexpr uint64_t c_bitMax = c_objectsPerChunk - 1;
    static constexpr uint64_t c_bit = 1;
    static_assert( c_atomicCount * c_objectsPerChunk >= TCapacity );


    std::array<Atomic, c_atomicCount> m_indexes{};

    static uint64_t pickIndex( Atomic& atomic ) noexcept
    {
        uint64_t oldValue = 0;
        uint64_t candidate = 0;
        uint64_t index = 0;
        do {
            oldValue = atomic.load();
            const uint64_t neg = ~oldValue;
            if ( neg == 0 ) { return c_invalidIndex; }
            index = static_cast<uint64_t>( std::countl_zero( neg ) );
            const uint64_t bitToSet = c_bitMax - index;
            const uint64_t bitMask = c_bit << bitToSet;
            assert( std::popcount( bitMask ) == 1 );
            candidate = oldValue | bitMask;
        } while ( !atomic.compare_exchange_strong( oldValue, candidate ) );
        return index;
    }

    static constexpr std::pair<size_t, size_t> split( size_t index ) noexcept
    {
        const uint64_t chunk = index >> 6;
        const uint64_t bit = c_bitMax - ( index & 0b111111 );
        const uint64_t bitMask = c_bit << bit;
        assert( std::popcount( bitMask ) == 1 );
        return { chunk, bitMask };
    }

public:
    [[nodiscard]]
    uint64_t next()
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

    void release( uint64_t index )
    {
        const auto [ chunk, bitMask ] = split( index );
        assert( ( m_indexes[ chunk ].load() & bitMask ) == bitMask );
        m_indexes[ chunk ].fetch_and( ~bitMask );
    }

    void releaseAll()
    {
        for ( auto& it : m_indexes ) {
            it.store( 0 );
        }
    }
};
