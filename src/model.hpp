#pragma once

#include "mesh.hpp"
#include "render_context.hpp"

#include <engine/math.hpp>
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
    std::array<math::vec3, 3> m_weapons{};
    std::array<math::vec3, 2> m_thrusterAfterglow{};
    uint32_t m_thrusterAfterglowCount = 0;

public:
    Buffer m_hull{};
    Buffer m_thruster{};
    Buffer m_wings{};
    Buffer m_tail{};
    Buffer m_intake{};
    ~Model() = default;
    Model() = default;
    Model( const Mesh&, Texture ) noexcept;

    math::vec3 weapon( uint32_t ) const;
    void render( const RenderContext& ) const;

};
