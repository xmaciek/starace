#pragma once

#include "mesh.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <renderer/renderer.hpp>

#include <filesystem>
#include <memory_resource>
#include <cstdint>
#include <vector>

class Model {
private:
    Texture m_texture{};
    std::vector<math::vec3> m_thrusters{};
    std::array<math::vec3, 3> m_weapons{};

public:
    Buffer m_hull{};
    Buffer m_engines{};
    Buffer m_wings{};
    Buffer m_elevators{};
    Buffer m_fins{};
    ~Model() = default;
    Model() = default;
    Model( const Mesh&, Texture ) noexcept;

    math::vec3 weapon( uint32_t ) const;
    std::vector<math::vec3> thrusters() const;
    void render( const RenderContext& ) const;

};
