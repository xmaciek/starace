#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <span>

Texture parseTexture( std::span<const uint8_t> );

void destroyTexture( Texture );
