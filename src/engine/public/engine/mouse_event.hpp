#pragma once

#include <glm/vec2.hpp>

#include <variant>

struct MouseMove : glm::vec2 {};
struct MouseClick : glm::vec2 {};
using MouseEvent = std::variant<std::monostate, MouseMove, MouseClick>;
