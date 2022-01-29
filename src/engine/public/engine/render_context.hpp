#pragma once

#include <engine/math.hpp>

class Renderer;
struct RenderContext {
    Renderer* renderer = nullptr;
    math::mat4 model = math::mat4( 1.0f );
    math::mat4 view = math::mat4( 1.0f );
    math::mat4 projection = math::mat4( 1.0f );

    math::mat4 camera3d = math::mat4( 1.0f );
    math::vec2 viewport{};
};
