#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <tuple>

template <typename TKey, typename TValue, size_t TCapacity>
class FixedMap {
    using size_type = std::size_t;
    using TKeyStorage = std::aligned_storage_t<sizeof(TKey), alignof(TKey)>;
    using TValueStorage = std::aligned_storage_t<sizeof(TValue), alignof(TValue)>;
    std::array<TKeyStorage, TCapacity> m_keyList{};
    std::array<TValueStorage, TCapacity> m_valueList{};
    size_type m_currentSize = 0;

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

    size_type size() const noexcept
    {
        return m_currentSize;
    }

    const TValue* operator [] ( const TKey& k ) const noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        const size_type dist = it - keyBegin();
        return valueBegin() + dist;
    }

    TValue* operator [] ( const TKey& k ) noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto it = std::lower_bound( keyBegin(), keyEnd(), k );
        if ( it == keyEnd() ) { return nullptr; }
        const size_type dist = it - keyBegin();
        return valueBegin() + dist;
    }

    std::tuple<TValue*, TValue*> equalRange( const TKey& k ) noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto [ begin, end ] = std::equal_range( keyBegin(), keyEnd(), k );
        if ( begin == end || begin == keyEnd() ) { return {}; }
        const size_type diffBegin = begin - keyBegin();
        const size_type diffEnd = end - keyBegin();
        return { valueBegin() + diffBegin, valueBegin() + diffEnd };
    }

    std::tuple<const TValue*, const TValue*> equalRange( const TKey& k ) const noexcept
    {
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
        auto [ begin, end ] = std::equal_range( keyBegin(), keyEnd(), k );
        if ( begin == end || begin == keyEnd() ) { return {}; }
        const size_type diffBegin = begin - keyBegin();
        const size_type diffEnd = end - keyBegin();
        return { valueBegin() + diffBegin, valueBegin() + diffEnd };
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

    void insert( TKey&& k, TValue&& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        const auto oldEnd = keyEnd();
        auto it = std::lower_bound( keyBegin(), oldEnd, k );
        new ( &m_keyList[ m_currentSize ] ) TKey( k );
        new ( &m_valueList[ m_currentSize ] ) TValue( v );
        m_currentSize += 1;
        if ( it == oldEnd ) {
            return;
        }

        const size_type dist = it - keyBegin();
        std::rotate( keyBegin() + dist, keyEnd() - 1, keyEnd() );
        std::rotate( valueBegin() + dist, valueEnd() - 1, valueEnd() );
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
    }

    void insert( const TKey& k, const TValue& v ) noexcept
    {
        assert( m_currentSize < TCapacity );
        const auto oldEnd = keyEnd();
        auto it = std::lower_bound( keyBegin(), oldEnd, k );
        new ( &m_keyList[ m_currentSize ] ) TKey( k );
        new ( &m_valueList[ m_currentSize ] ) TValue( v );
        m_currentSize += 1;
        if ( it == oldEnd ) {
            return;
        }

        const size_type dist = it - keyBegin();
        std::rotate( keyBegin() + dist, keyEnd() - 1, keyEnd() );
        std::rotate( valueBegin() + dist, valueEnd() - 1, valueEnd() );
        assert( std::is_sorted( keyBegin(), keyEnd() ) );
    }
};
