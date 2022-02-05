#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <vector>
#include <memory_resource>

Texture parseTexture( std::pmr::vector<uint8_t>&& );

void destroyTexture( Texture );
