#pragma once

#include "renderer.hpp"

#include <glm/mat4x4.hpp>

struct RenderContext {
    Renderer* renderer = nullptr;
    glm::mat4 model = glm::mat4( 1.0f );
    glm::mat4 view = glm::mat4( 1.0f );
    glm::mat4 projection = glm::mat4( 1.0f );

    RenderContext() = default;
};
