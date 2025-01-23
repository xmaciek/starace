#pragma once

#include <vulkan/vulkan.h>

#define DECL_FUNCTION( name ) inline constinit PFN_##name name = nullptr;
#define DECL_FUNCTION_OPTIONAL( name ) inline constinit PFN_##name name = nullptr;
#include "vk.def"
