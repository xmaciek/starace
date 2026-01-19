#pragma once

#include "mesh.hpp"
#include "render_context.hpp"

#include <engine/math.hpp>
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>
#include <renderer/renderer.hpp>
#include <shared/stack_vector.hpp>

#include <filesystem>
#include <memory_resource>
#include <cstdint>
#include <vector>

class Model {
private:
    Texture m_texture{};
    Hardpoints m_hardpoints{};
    StackVector<math::vec3, 2> m_thrusterAfterglow{};

public:
    Buffer m_hull{};
    Buffer m_thruster{};
    Buffer m_wings{};
    Buffer m_tail{};
    Buffer m_intake{};
    ~Model() = default;
    Model() = default;
    Model( const Mesh&, Texture ) noexcept;

    const Hardpoints& hardpoints() const { return m_hardpoints; }

    void render( const RenderContext& ) const;

};
