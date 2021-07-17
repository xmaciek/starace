#pragma once

#include <renderer/texture.hpp>

#include <cstdint>
#include <string_view>

Texture loadDefault();
Texture loadTexture( std::string_view );
void destroyTexture( Texture );
