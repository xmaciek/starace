#pragma once


#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>

#include <string>
#include <memory_resource>

using LocTable = FixedMap<Hash::value_type, std::pmr::u32string, 64>;
