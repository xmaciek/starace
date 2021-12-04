#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

template <typename TKey, typename TValue, size_t TCapacity>
class FixedMap {
    using TKeyStorage = std::aligned_storage_t<sizeof(TKey), alignof(TKey)>;
    using TValueStorage = std::aligned_storage_t<sizeof(TValue), alignof(TValue)>;
    std::array<TKeyStorage, TCapacity> m_keyList{};
    std::array<TValueStorage, TCapacity> m_valueList{};
    std::size_t m_currentSize = 0;

    auto keyBegin() const noexcept { return reinterpret_cast<const TKey*>( m_keyList.begin() ); }
    auto keyBegin() noexcept { return reinterpret_cast<TKey*>( m_keyList.begin() ); }
    auto keyEnd() const noexcept { return keyBegin() + m_currentSize; }
    auto keyEnd() noexcept { return keyBegin() + m_currentSize ; }
    auto valueBegin() const noexcept { return reinterpret_cast<const TValue*>( m_valueList.begin() ); }
    auto valueBegin() noexcept { return reinterpret_cast<TValue*>( m_valueList.begin() ); }
    auto valueEnd() const noexcept { return valueBegin() + m_currentSize; }
    auto valueEnd() noexcept { return valueBegin() + m_currentSize; }

public:
    ~FixedMap() noexcept
    {
        std::destroy( valueBegin(), valueEnd() );
        std::destroy( keyBegin(), keyEnd() );
    }

    FixedMap() noexcept = default;
    const TValue* operator [] ( const TKey& k ) const noexcept
    {
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        if ( *it != k ) { return nullptr; }
        const size_t dist = it - keyBegin();
        return valueBegin() + dist;
    }

    TValue* operator [] ( const TKey& k ) noexcept
    {
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        if ( *it != k ) { return nullptr; }
        const size_t dist = it - keyBegin();
        return valueBegin() + dist;
    }

    void pushBack( const TKey& k, const TValue& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        assert( m_currentSize == 0 || *reinterpret_cast<TKey*>(&m_keyList[ m_currentSize - 1 ]) < k );
        new ( &m_keyList[ m_currentSize ] ) TKey ( k );
        new ( &m_valueList[ m_currentSize ] ) TValue ( v );
        m_currentSize += 1;
    }

    void pushBack( TKey&& k, TValue&& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        assert( m_currentSize == 0 || *reinterpret_cast<TKey*>(&m_keyList[ m_currentSize - 1 ]) < k );
        new ( &m_keyList[ m_currentSize ] ) TKey ( k );
        new ( &m_valueList[ m_currentSize ] ) TValue ( v );
        m_currentSize += 1;
    }

};
