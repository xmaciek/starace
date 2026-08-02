#pragma once

#include <algorithm>

auto unique_add( auto& container, auto value )
{
    auto it = std::ranges::find( container, value );
    if ( it != container.end() ) return it;
    container.emplace_back( value );
    return container.end() - 1;
}
