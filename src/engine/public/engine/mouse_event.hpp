#pragma once

#include <engine/math.hpp>

#include <variant>

struct MouseMove : math::vec2 {};
struct MouseClick : math::vec2 {};
using MouseEvent = std::variant<std::monostate, MouseMove, MouseClick>;
