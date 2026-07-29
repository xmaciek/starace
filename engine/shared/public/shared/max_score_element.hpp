#pragma once

#include <cstddef>
#include <type_traits>
#include <limits>

template <typename TContainer, typename TCMP, typename TScoreType = size_t>
requires ( std::is_invocable_r_v<TScoreType, TCMP, typename TContainer::value_type> )
inline auto maxScoreElement( const TContainer& container, TCMP&& cmp )
{
    TScoreType score = std::numeric_limits<TScoreType>::min();
    auto ret = container.begin();
    auto end = container.end();
    for ( auto it = ret; it != end; ++it ) {
        TScoreType s = cmp( *it );
        if ( s <= score ) continue;
        ret = it;
        score = s;
    }
    return ret;
}


template <typename TContainer, typename TCMP, typename TScoreType = size_t>
requires ( std::is_invocable_r_v<TScoreType, TCMP, typename TContainer::value_type> )
inline auto minScoreElement( const TContainer& container, TCMP&& cmp )
{
    TScoreType score = std::numeric_limits<TScoreType>::max();
    auto ret = container.begin();
    auto end = container.end();
    for ( auto it = ret; it != end; ++it ) {
        TScoreType s = cmp( *it );
        if ( s >= score ) continue;
        ret = it;
        score = s;
    }
    return ret;
}
