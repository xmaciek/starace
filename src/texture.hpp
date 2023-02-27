#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <vector>
#include <memory_resource>
#include <span>

Texture parseTexture( std::pmr::vector<uint8_t>&& );
Texture parseTexture( std::span<const uint8_t> );

void destroyTexture( Texture );
