#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>

template <typename T, std::size_t CAPACITY>
requires ( CAPACITY > 0
    && std::is_trivially_constructible_v<T>
    && std::is_trivially_copyable_v<T>
    && std::is_trivially_destructible_v<T>
)
class StackVector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference_type = value_type&;
    using const_reference_type = const value_type&;

private:
    size_type m_size = 0;
    std::array<value_type, CAPACITY> m_data;

public:
    StackVector() = default;
    StackVector( size_type s ) : m_size{ s } { assert( s <= capacity() ); }

    [[nodiscard]]
    bool empty() const { return m_size == 0; };
    size_type size() const { assert( m_size <= capacity() ); return m_size; }
    constexpr size_type capacity() const { return CAPACITY; }
    void clear() { m_size = 0; }

    reference_type push_back( const T& t ) { assert( m_size < capacity() ); return m_data[ m_size++ ] = t; }
    reference_type emplace_back( auto... t ) { assert( m_size < capacity() ); return m_data[ m_size++ ] = value_type( t... ); }

    reference_type front() { assert( size() ); return *begin(); }
    const_reference_type front() const { assert( size() ); return *begin(); }
    reference_type back() { assert( size() ); return *( end() - 1 ); }
    const_reference_type back() const { assert( size() ); return *( end() - 1 ); }
    void resize( std::size_t s ) { assert( s <= capacity() ); m_size = s; }

    auto begin() { return m_data.begin(); }
    auto begin() const { return m_data.begin(); }
    auto end() { return begin() + size(); }
    auto end() const { return begin() + size(); }
    value_type* data() { return m_data.data(); }
    const value_type* data() const { return m_data.data(); }
    reference_type operator [] ( std::size_t i ) { assert( i < size() ); return m_data[ i ]; }
    const_reference_type operator [] ( std::size_t i ) const { assert( i < size() ); return m_data[ i ]; }

    void swap( StackVector& rhs )
    {
        std::swap( m_size, rhs.m_size );
        std::swap( m_data, rhs.m_data );
    }
};
