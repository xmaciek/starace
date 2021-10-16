#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class Renderer;
struct RenderContext {
    Renderer* renderer = nullptr;
    glm::mat4 model = glm::mat4( 1.0f );
    glm::mat4 view = glm::mat4( 1.0f );
    glm::mat4 projection = glm::mat4( 1.0f );

    glm::mat4 camera3d = glm::mat4( 1.0f );
    glm::vec2 viewport{};
};
