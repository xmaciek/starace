#pragma once

#include <glm/mat4x4.hpp>

struct UpdateContext {
    glm::mat4 camera{ 1.0f };
    glm::vec2 viewport{};
    float deltaTime = 0.0f;
};
