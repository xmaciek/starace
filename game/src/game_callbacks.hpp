#pragma once

#include <functional>
#include <string_view>
#include <unordered_map>

inline std::unordered_map<std::string_view, std::function<void()>> g_gameCallbacks{};
