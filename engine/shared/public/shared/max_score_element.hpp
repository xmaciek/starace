#pragma once

#include <cstddef>
#include <type_traits>

template <typename TContainer, typename TCMP>
requires ( std::is_invocable_r_v<size_t, TCMP, typename TContainer::value_type> )
inline auto maxScoreElement( const TContainer& container, TCMP&& cmp )
{
    size_t score = 0;
    auto ret = container.begin();
    auto end = container.end();
    for ( auto it = ret; it != end; ++it ) {
        size_t s = cmp( *it );
        if ( s <= score ) continue;
        ret = it;
        score = s;
    }
    return ret;
}
