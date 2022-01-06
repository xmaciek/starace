#pragma once

#include <string>

class Model;

struct ModelProto {
    std::u32string name{};
    std::string model_file{};
    std::string model_texture{};
    Model* model = nullptr;
    float scale = 1.0f;
};
