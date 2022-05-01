#pragma once

#include <string>
#include <memory_resource>

class Model;

struct ModelProto {
    std::pmr::u32string name{};
    std::string model_file{};
    std::string model_texture{};
    Model model{};
    float scale = 1.0f;
};
