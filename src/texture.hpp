#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <string_view>
#include <vector>
#include <memory_resource>

Texture loadDefault();
Texture loadTexture( std::string_view );
Texture loadTexture( std::pmr::vector<uint8_t>&& );

void destroyTexture( Texture );
