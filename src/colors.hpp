#pragma once

#include <glm/vec4.hpp>

#include <array>

namespace color {

constexpr glm::vec4 blaster{ 0.3f, 0.8f, 1.0f, 1.0f };
constexpr glm::vec4 dodgerBlue{ 0.118f, 0.565f, 1.0f, 1.0f };
constexpr glm::vec4 orchid{ 0.855f, 0.439f, 0.839f, 1.0f };
constexpr glm::vec4 white{ 1.0f, 1.0f, 1.0f, 1.0f };
constexpr glm::vec4 yellowBlaster{ 1.0f, 0.8f, 0.1f, 1.0f};
constexpr glm::vec4 yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
constexpr glm::vec4 pause{ 0.1f, 0.4f, 0.9f, 0.8f };
constexpr glm::vec4 winScreen{ 0.0275f, 1.0f, 0.075f, 1.0f };
constexpr glm::vec4 crimson{ 0.863f, 0.078f,  0.234f, 1.0f };
constexpr glm::vec4 lightSkyBlue{ 0.529f, 0.807f, 0.98f, 1.0f };
constexpr glm::vec4 lightSteelBlue{ 0.69f, 0.769f, 0.87f, 1.0f };

} // namesapce color

using ColorScheme = std::array<glm::vec4, 4>;

namespace colorscheme {

constexpr ColorScheme ion{
    glm::vec4{ 0.0f, 0.7f, 1.0f, 1.0f },
    glm::vec4{ 0.0f, 0.0f, 0.5f, 0.0f },
    glm::vec4{ 0.0f, 0.75f, 1.0f, 0.6f },
    glm::vec4{ 0.0f, 0.0f, 0.5f, 0.0f },
};

constexpr ColorScheme chartreuse{
    glm::vec4{ 0.9f, 1.0f, 0.0f, 1.0f },
    glm::vec4{ 0.25f, 0.5f, 0.0f, 0.0f },
    glm::vec4{ 0.6f, 1.0f, 0.0f, 0.618f },
    glm::vec4{ 0.1f, 0.3f, 0.0f, 0.0f },
};

constexpr ColorScheme coral{
    glm::vec4{ 1.0f, 0.5f, 0.313f, 1.0f },
    glm::vec4{ 0.5f, 0.0f, 0.0f, 0.0f },
    glm::vec4{ 1.0f, 0.894f, 0.710f, 0.618f },
    glm::vec4{ 0.5f, 0.161f, 0.0f, 0.0f },
};

} // namespace colorscheme
