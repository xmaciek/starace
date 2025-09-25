#pragma once

#include "model.hpp"

#include <string>
#include <memory_resource>


struct ModelProto {
    std::pmr::u32string name{};
    Model model{};
};
