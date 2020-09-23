#pragma once

#include <string>

struct ModelProto {
    std::string name{ "Unnamed Jet" };
    std::string model_file{};
    std::string model_texture{};
    float scale = 1.0f;
};
