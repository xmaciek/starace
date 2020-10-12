#pragma once

#include "render_context.hpp"
#include <renderer/buffer.hpp>
#include <renderer/texture.hpp>

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

struct UV {
    float u = 0;
    float v = 0;
};

struct Face {
    std::vector<glm::vec3> vertex{};
    std::vector<UV> texcoord{};
    double normal[ 3 ]{};
};

class Model {
private:
    mutable Buffer m_vertices{};
    mutable Buffer m_normals{};
    mutable Buffer m_uv{};
    Texture m_textureID{};
    std::vector<Face> m_faces{};
    std::vector<glm::vec3> m_thrusters{};
    glm::vec3 m_weapons[ 3 ]{};

public:
    ~Model();
    Model() = default;

    glm::vec3 weapon( uint32_t ) const;
    std::vector<glm::vec3> thrusters() const;
    void bindTexture( Texture );
    void calculateNormal();
    void render( RenderContext ) const;
    void draw() const;
    void drawWireframe();
    void loadOBJ( const char* filename );
    void normalizeSize();
    void scale( float scale );
};
