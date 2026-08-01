#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

// NOTE: drop this the moment std::inplace_vector exists in all compilers

template <typename T, std::size_t CAPACITY>
requires ( CAPACITY > 0 )
class StackVector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference_type = value_type&;
    using const_reference_type = const value_type&;

private:
    struct Storage {
        alignas( alignof( value_type ) ) std::byte data[ sizeof( value_type ) ];
        value_type* emplace( auto... v )
        {
            return std::construct_at<value_type>( std::launder( reinterpret_cast<value_type*>( this ) ), std::forward<decltype(v)>( v )... );
        }
        operator value_type* () { return reinterpret_cast<value_type*>( this ); }
        operator const value_type* () const { return reinterpret_cast<const value_type*>( this ); }
        value_type& operator * () { return *reinterpret_cast<value_type*>( this ); }
        const value_type& operator * () const { return *reinterpret_cast<const value_type*>( this ); }
    };
    size_type m_size = 0;
    std::array<Storage, CAPACITY> m_data;

public:
    ~StackVector()
    {
        clear();
    }
    StackVector() = default;
    StackVector( size_type s )
    {
        assert( s <= capacity() );
        resize( s );
    }

    StackVector( const StackVector& r ) requires ( std::is_copy_constructible_v<value_type> )
    {
        if constexpr ( std::is_trivially_copy_constructible_v<value_type> ) {
            std::ranges::copy( r, begin() );
            m_size = r.m_size;
        }
        else {
            std::ranges::for_each( r, [this]( auto& it ) { this->emplace_back( it ); } );
        }
    }

    StackVector( StackVector&& r ) requires ( std::is_move_constructible_v<value_type> )
    {
        if constexpr ( std::is_trivially_move_constructible_v<value_type> ) {
            std::ranges::move( r, begin() );
            m_size = r.m_size;
        }
        else {
            std::ranges::for_each( r, [this]( auto& it ) { this->emplace_back( std::move( it ) ); } );
        }
        r.clear();
    }

    StackVector& operator = ( const StackVector& r ) requires ( std::is_copy_assignable_v<value_type> )
    {
        clear();
        if constexpr ( std::is_trivially_copy_assignable_v<value_type> ) {
            std::ranges::copy( r, begin() );
            m_size = r.m_size;
        }
        else {
            std::ranges::for_each( r, [this]( auto& it ) { this->emplace_back() = it; } );
        }
        return *this;
    }

    StackVector& operator = ( StackVector&& r ) requires ( std::is_move_assignable_v<value_type> )
    {
        clear();
        if constexpr ( std::is_trivially_move_assignable_v<value_type> ) {
            std::ranges::move( r, begin() );
            m_size = r.m_size;
        }
        else {
            std::ranges::for_each( r, [this]( auto& it ) { this->emplace_back() = std::move( it ); } );
        }
        r.clear();
        return *this;
    }

    [[nodiscard]]
    bool empty() const { return m_size == 0; };
    size_type size() const { assert( m_size <= capacity() ); return m_size; }
    constexpr size_type capacity() const { return CAPACITY; }
    void clear() { std::destroy_n( data(), std::exchange( m_size, 0 ) ); }

    reference_type emplace_back( auto... t ) { assert( m_size < capacity() ); return *m_data[ m_size++ ].emplace( std::forward<decltype(t)>( t )... ); }
    reference_type push_back( const T& t ) requires ( std::is_copy_constructible_v<value_type> ) { return emplace_back( t ); }

    reference_type front() { assert( size() ); return *begin(); }
    const_reference_type front() const { assert( size() ); return *begin(); }
    reference_type back() { assert( size() ); return *( end() - 1 ); }
    const_reference_type back() const { assert( size() ); return *( end() - 1 ); }
    void resize( size_type s )
    {
        assert( s <= capacity() );
        [[maybe_unused]]
        auto oldSize = std::exchange( m_size, s );
        if ( oldSize > m_size ) {
            std::destroy_n<value_type*>( data() + m_size, oldSize - m_size );
        }
        else if constexpr ( !std::is_trivially_constructible_v<value_type> ) {
            std::for_each( m_data.begin() + oldSize, m_data.begin() + m_size, []( auto& it ){ it.emplace(); } );
        }
    }

    value_type* begin() { return *m_data.begin(); }
    const value_type* begin() const { return *m_data.begin(); }
    value_type* end() { return begin() + size(); }
    const value_type* end() const { return begin() + size(); }
    value_type* data() { return *m_data.data(); }
    const value_type* data() const { return *m_data.data(); }
    reference_type operator [] ( size_type i ) { assert( i < size() ); return *m_data[ i ]; }
    const_reference_type operator [] ( size_type i ) const { assert( i < size() ); return *m_data[ i ]; }

};
